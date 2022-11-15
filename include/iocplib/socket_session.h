#ifndef __SOCK_SESSION_H__
#define __SOCK_SESSION_H__
#include "win_sock.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPort;

	class SocketSession
		: public WinSock
		, public OverlappedEventInterface
	{
	public:
		SocketSession();
		virtual ~SocketSession() {}

		void OnAccept(IoCompletionPort* iocp);

		virtual void OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) override;

	private:
		IoCompletionPort* iocp_{ nullptr };
	};
}

#endif
