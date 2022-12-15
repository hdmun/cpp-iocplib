#ifndef __IO_COMPLETION_PORT_H__
#define __IO_COMPLETION_PORT_H__
#include "../winsocklib/win_sock.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPortWorker
	{
	public:
		IoCompletionPortWorker(IoCompletionPortWorker&) = delete;
		IoCompletionPortWorker operator= (IoCompletionPortWorker&) = delete;

		IoCompletionPortWorker(HANDLE iocp_handle)
			: iocp_handle_(iocp_handle)
			, thread_(&IoCompletionPortWorker::Run, this)
		{
		}

		virtual ~IoCompletionPortWorker() {}

		void Run()
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
				context.callback->OnCompleteOverlappedIO(context.data, dwError, dwNumberOfBytesTransferred, completionKey);
			}
		}

		void Join()
		{
			thread_.join();
		}

	private:
		HANDLE iocp_handle_{ nullptr };
		std::thread thread_;
	};

	class IoCompletionPort
	{
	public:
		IoCompletionPort(IoCompletionPort&) = delete;
		IoCompletionPort operator= (IoCompletionPort&) = delete;

		IoCompletionPort() {}
		virtual ~IoCompletionPort() {}

		HANDLE handle() const {
			return handle_;
		}

		bool Create(uint32_t io_thread = 0U)
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

		void Close()
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

		void Attach(HANDLE socket)
		{
			// 소켓을 IOCP 핸들에 연결
			if (!::CreateIoCompletionPort(socket, handle_, 0, 0)) {
				__noop;  // exception?
				return;
			}

			attached_sockets_++;
		}

		void Detach()
		{
			attached_sockets_--;
		}

	private:
		HANDLE handle_{ nullptr };

		std::mutex worker_lock_;
		std::vector< std::unique_ptr<IoCompletionPortWorker> > workers_;

		inline static std::atomic<uint32_t> attached_sockets_{ 0 };
	};
}


#endif