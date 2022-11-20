#include "pch.h"
#include "../socket_session.h"
#include "../io_completion_port.h"
#include "../overlapped.h"

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
					int received = session_->Recv(reinterpret_cast<char*>(buffer_), kSocketBufferSize, 0);
					if (received == SOCKET_ERROR) {
						int err = ::WSAGetLastError();
						if (err != WSAEWOULDBLOCK) {
							dwError = static_cast<DWORD>(err);
						}
						break;  // break while
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

	SocketSession::SocketSession()
		: receiver_(this)
	{
	}

	void SocketSession::OnAccept(IoCompletionPort* iocp)
	{
		WinSock::SetNonBlocking(true);

		iocp_ = iocp;
		iocp_->Attach(reinterpret_cast<HANDLE>(handle()));

		// todo: nagle option ²ô±â

		receiver_.BeginReceive();
	}

	void SocketSession::OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
	{
		switch (data.type) {
		case eOverlappedType::Recv:
			receiver_.OnReceive(dwError, dwBytesTransferred);
			break;
		case eOverlappedType::Send:
			break;
		default:
			// error
			break;
		}
	}
}