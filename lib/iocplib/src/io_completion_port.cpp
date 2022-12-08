#include "pch.h"
#include "../io_completion_port.h"
#include "../overlapped.h"

namespace iocplib {
	IoCompletionPortWorker::IoCompletionPortWorker(HANDLE iocp_handle)
		: iocp_handle_(iocp_handle)
		, thread_(&IoCompletionPortWorker::Run, this)
	{
	}

	void IoCompletionPortWorker::Run()
	{
		while (true)
		{
			DWORD dwNumberOfBytesTransferred = 0;
			ULONG_PTR completionKey;
			LPOVERLAPPED lpOverlapped = nullptr;

			BOOL bSuccess = ::GetQueuedCompletionStatus(
				iocp_handle_,
				&dwNumberOfBytesTransferred,
				&completionKey,
				&lpOverlapped,
				INFINITE
			);

			if (!lpOverlapped) {
				break;  // null 이면 스레드 종료 신호로 처리
			}

			DWORD dwError = bSuccess ? 0 : ::GetLastError();
			OverlappedContext context = *reinterpret_cast<OverlappedContext*>(lpOverlapped);
			context.callback->OnComplete(context.data, dwError, dwNumberOfBytesTransferred, completionKey);
		}
	}

	void IoCompletionPortWorker::Join()
	{
		thread_.join();
	}


	std::atomic<uint32_t> IoCompletionPort::attached_sockets_{ 0 };

	bool IoCompletionPort::Create(uint32_t io_thread)
	{
		// IOCP 커널 오브젝트를 생성
		handle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		if (!handle_) {
			return false;
		}

		if (io_thread == 0U) {
			SYSTEM_INFO si{ 0, };
			::GetSystemInfo(&si);
			io_thread = si.dwNumberOfProcessors * 2;
		}

		{
			std::lock_guard<std::mutex> lock(worker_lock_);
			workers_.reserve(io_thread);
			for (uint32_t i = 0U; i < io_thread; i++) {
				workers_.push_back(std::make_unique<IoCompletionPortWorker>(handle_));
			}
		}

		return true;
	}

	void IoCompletionPort::Close()
	{
		// wait socket handles
		while (attached_sockets_ > 0) {
			::YieldProcessor();
		}

		// wait worker thread
		{
			std::lock_guard<std::mutex> lock(worker_lock_);
			for (int i = 0; i < workers_.size(); i++) {
				::PostQueuedCompletionStatus(handle_, 0, 0, nullptr);
			}

			for (auto& worker : workers_) {
				worker->Join();
			}
		}
	}

	void IoCompletionPort::Attach(HANDLE socket)
	{
		// 소켓을 IOCP 핸들에 연결
		if (!::CreateIoCompletionPort(socket, handle_, 0, 0)) {
			__noop;  // exception?
			return;
		}

		attached_sockets_++;
	}

	void IoCompletionPort::Detach()
	{
		attached_sockets_--;
	}
}
