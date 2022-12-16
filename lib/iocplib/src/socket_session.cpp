#include "pch.h"
#include "../socket_session.h"
#include "../io_completion_port.h"
#include "../overlapped.h"
#include "../socket_buffer.h"

namespace iocplib {
	SessionReceiver::SessionReceiver()
		: zero_byet_recv_(false)
	{
		overlapped_context_.data.type = eOverlappedType::Recv;
	}

	SessionReceiver::~SessionReceiver()
	{
		session_.reset();
	}

	void SessionReceiver::OnAccept(std::weak_ptr<SocketSession> session)
	{
		session_ = session;
		overlapped_context_.callback = session;
	}

	int32_t SessionReceiver::BeginReceive()
	{
		std::lock_guard<std::recursive_mutex> lock(lock_);

		WSABUF buf;
		buf.buf = nullptr;
		buf.len = 0;

		zero_byet_recv_ = true;

		DWORD dwReceived = 0;
		DWORD dwFlags = 0;
		int32_t err = ::WSARecv(session_.lock()->handle(), &buf, 1UL, &dwReceived, &dwFlags, &overlapped_context_, nullptr);
		if (err == SOCKET_ERROR) {
			err = ::WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				return err;
			}
		}

		return 0;
	}

	void SessionReceiver::OnReceive(DWORD dwError, DWORD dwBytesTransferred)
	{
		do {
			// 에러 체크
			if (dwError) {
				dwError = OnError(dwError);
				break;  // break do-while
			}

			std::lock_guard<std::recursive_mutex> lock(lock_);

			if (zero_byet_recv_) {
				zero_byet_recv_ = false;

				// `WSAEWOULDBLOCK` 리턴 될 때 까지 recv 시도
				auto session = session_.lock();
				while (true) {
					int received = session->TryRecv(reinterpret_cast<char*>(buffer_), winsocklib::kSocketBufferSize, 0);
					if (received == SOCKET_ERROR) {
						int err = ::WSAGetLastError();
						if (err == WSAEWOULDBLOCK) {
							break;
						}

						dwError = err;
						break;
					}

					if (received == 0) {
						recv_error_ = WSAECONNRESET; // disconnect
						break;
					}

					session->OnReceivePacket(buffer_, received);
				}

				if (dwError == ERROR_SUCCESS) {
					dwError = BeginReceive();
				}

				if (dwError) {
					dwError = OnError(dwError);
				}
			}
		} while (false);

		if (dwError) {
			// 수신 실패 에러 처리
		}
	}

	int32_t SessionReceiver::OnError(DWORD dwError)
	{
		switch (dwError) {
		case ERROR_SUCCESS:
			break;

		case ERROR_OPERATION_ABORTED:
		case ERROR_NO_SYSTEM_RESOURCES:
		case WSAENOBUFS:
			return BeginReceive(); // 한 번 더 다시 시도

		default:
			recv_error_ = dwError;
			break;
		}
		return dwError;
	}

	SessionSender::SessionSender()
	{
		overlapped_context_.data.type = eOverlappedType::Send;
	}

	SessionSender::~SessionSender()
	{
		session_.reset();
	}

	void SessionSender::OnAccept(std::weak_ptr<SocketSession> session)
	{
		session_ = session;
		overlapped_context_.callback = session;
	}

	void SessionSender::OnSend(DWORD dwError, DWORD dwBytesTransferred)
	{
		std::lock_guard<std::recursive_mutex> lock(lock_);

		if (dwError == ERROR_SUCCESS) {
			// 전송된 만큼 sending_packets_ 버퍼 정리
			dwBytesTransferred += sending_offset_;
			sending_offset_ = 0;

			while (dwBytesTransferred && !sending_packets_.empty()) {
				const auto& sending = sending_packets_.front();
				if (dwBytesTransferred >= sending->GetBufferSize()) {
					dwBytesTransferred -= sending->GetBufferSize();
					sending_packets_.pop_front();
				}
				else {
					sending_offset_ = dwBytesTransferred;
					break;
				}
			}
		}

		// 전송 시도
		dwError = BeginSend();
		switch (dwError) {
		case ERROR_SUCCESS:
			break;

		case ERROR_OPERATION_ABORTED:
		case ERROR_NO_SYSTEM_RESOURCES:
		case WSAENOBUFS:
			dwError = BeginSend(); // 한 번 더 다시 시도
			break;

		default:
			break;
		}

		if (dwError) {
			send_error_ = dwError;
			post_send_signal = false;

			// todo: 에러 전파
		}
	}

	void SessionSender::SendAsync(uint8_t* data, uint32_t size)
	{
		std::lock_guard<std::recursive_mutex> lock(lock_);

		auto session = session_.lock();
		if (session == nullptr) {
			return;
		}

		// send_queue_에 버퍼 데이터를 생성하고
		send_queue_.push(SocketBuffer::Allocate(data, size));

		// completion port에 signal을 날려주자
		PostCompletionPortSignal(session->GetIocpHandle());
	}

	int32_t SessionSender::BeginSend()
	{
		std::lock_guard<std::recursive_mutex> lock(lock_);

		if (send_queue_.empty()) {
			post_send_signal = false;
			return ERROR_SUCCESS;
		}

		// send_queue_ 데이터 sending_packets_로 이동
		while (!send_queue_.empty()) {
			sending_packets_.push_back(std::move(send_queue_.front()));
			send_queue_.pop();
		}

		std::vector<WSABUF> wsabufs;
		wsabufs.reserve(sending_packets_.size());
		for (const auto& sending : sending_packets_) {
			WSABUF buf = { 0, };
			buf.buf = reinterpret_cast<char*>(sending->buffer());
			buf.len = static_cast<ULONG>(sending->GetBufferSize());
			wsabufs.push_back(buf);
		}

		wsabufs[0].buf += sending_offset_;
		wsabufs[0].len -= sending_offset_;

		DWORD dwBufferCount = static_cast<DWORD>(wsabufs.size());
		DWORD dwSent = 0U;
		DWORD dwFlags = 0U;
		overlapped_context_.Offset = 0U;
		int ret = ::WSASend(session_.lock()->handle(), wsabufs.data(), dwBufferCount, &dwSent, dwFlags, &overlapped_context_, nullptr);
		if (ret == SOCKET_ERROR) {
			int err = ::WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				// todo: 실패 했으니 버퍼 원상 복구
				return err;
			}
		}

		return ERROR_SUCCESS;
	}

	void SessionSender::PostCompletionPortSignal(HANDLE iocp_handle)
	{
		std::lock_guard<std::recursive_mutex> lock(lock_);

		if (!post_send_signal) {
			if (!::PostQueuedCompletionStatus(iocp_handle, 0, 0, &overlapped_context_)) {

				throw WinSockException("", GetLastError());
			}

			post_send_signal = true;
		}
	}

	SocketSession::SocketSession()
	{
	}

	HANDLE SocketSession::GetIocpHandle() const
	{
		return iocp_->handle();
	}

	void SocketSession::OnAccept(IoCompletionPort* iocp)
	{
		winsocklib::WinSock::SetNonBlocking(true);
		winsocklib::WinSock::SetNagleAlogrithm(false);

		iocp_ = iocp;
		iocp_->Attach(reinterpret_cast<HANDLE>(handle()));

		receiver_.OnAccept(weak_from_this());
		sender_.OnAccept(weak_from_this());

		uint32_t error = receiver_.BeginReceive();
	}

	void SocketSession::OnCompleteOverlappedIO(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
	{
		switch (data.type) {
		case eOverlappedType::Recv:
			receiver_.OnReceive(dwError, dwBytesTransferred);
			break;
		case eOverlappedType::Send:
			sender_.OnSend(dwError, dwBytesTransferred);
			break;
		default:
			// error
			break;
		}
	}
}