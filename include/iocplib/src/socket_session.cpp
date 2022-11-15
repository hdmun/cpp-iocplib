#include "pch.h"
#include "../socket_session.h"
#include "../io_completion_port.h"
#include "../overlapped.h"

namespace iocplib {
	SocketSession::SocketSession()
	{
	}

	void SocketSession::OnAccept(IoCompletionPort* iocp)
	{
		iocp_ = iocp;
		iocp_->Attach(reinterpret_cast<HANDLE>(handle()));
	}

	void SocketSession::OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
	{
		switch (data.type) {
		case eOverlappedType::Recv:
			break;
		case eOverlappedType::Send:
			break;
		default:
			// error
			break;
		}
	}
}