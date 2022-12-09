#include "pch.h"
#include "../socket_session.h"
#include "../io_completion_port.h"
#include "../overlapped.h"
#include "../socket_buffer.h"

namespace iocplib {
	SessionReceiver::SessionReceiver(SocketSession* session)
		: session_(session)
	{
		overlapped_context_.data.type = eOverlappedType::Recv;
		overlapped_context_.callback = session_;
	}

	SessionReceiver::~SessionReceiver()
	{
		session_ = nullptr;
	}

	int SessionReceiver::BeginReceive()
	{
		WSABUF buf;
		buf.buf = nullptr;
		buf.len = 0;

		overlapped_context_.zero_byet_recv = true;

		DWORD dwReceived = 0;
		DWORD dwFlags = 0;
		int err = ::WSARecv(session_->handle(), &buf, 1UL, &dwReceived, &dwFlags, &overlapped_context_, nullptr);
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
			if (dwError) {
				break;
			}

			if (overlapped_context_.zero_byet_recv) {
				overlapped_context_.zero_byet_recv = false;
				while (true) {
					int received = session_->Recv(reinterpret_cast<char*>(buffer_), winsocklib::kSocketBufferSize, 0);
					if (received < 0) {
						// WSAEWOULDBLOCK
						break;
					}

					if (received == 0) {
						// disconnect
						break;
					}

					session_->OnReceivePacket(buffer_, received);
				}
				break;  // break do-while
			}

			if (dwBytesTransferred == 0) {
				__noop;
			}
		} while (false);

		// dwError


		BeginReceive();
	}

	SessionSender::SessionSender(SocketSession* session)
		: session_(session)
	{
		overlapped_context_.data.type = eOverlappedType::Send;
		overlapped_context_.callback = session_;
	}

	SessionSender::~SessionSender()
	{
		session_ = nullptr;
	}

	void SessionSender::SendAsync(uint8_t* data, uint32_t size)
	{
		// send_queue_에 버퍼 데이터를 생성하고
		send_queue_.push(SocketBuffer::Allocate(data, size));

		// completion port에 signal을 날려주자
		PostCompletionPortSignal(session_->GetIocpHandle());
	}

	int SessionSender::BeginSend()
	{
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
		int ret = ::WSASend(session_->handle(), wsabufs.data(), dwBufferCount, &dwSent, dwFlags, &overlapped_context_, nullptr);
		if (ret == SOCKET_ERROR) {
			int err = ::WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				
				return err;
			}
		}

		return ERROR_SUCCESS;
	}

	void SessionSender::OnSend(DWORD dwError, DWORD dwBytesTransferred)
	{
		if (dwError > 0) {
			return;
		}

		// 전송된 만큼 sending_packets_ 정리
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

		// send_queue_ 전송 시 wsabuf_도
		BeginSend();
	}

	void SessionSender::PostCompletionPortSignal(HANDLE iocp_handle)
	{
		if (!post_send_signal) {
			if (!::PostQueuedCompletionStatus(iocp_handle, 0, 0, &overlapped_context_)) {
				// GetLastError();
				// throw exception
			}

			post_send_signal = true;
		}
	}

	SocketSession::SocketSession()
		: receiver_(this)
		, sender_(this)
	{
	}

	HANDLE SocketSession::GetIocpHandle() const
	{
		return iocp_->handle();
	}

	void SocketSession::OnAccept(IoCompletionPort* iocp)
	{
		winsocklib::WinSock::SetNonBlocking(true);

		iocp_ = iocp;
		iocp_->Attach(reinterpret_cast<HANDLE>(handle()));

		// todo: nagle option 끄기

		receiver_.BeginReceive();
	}

	void SocketSession::OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
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