#ifndef __SOCK_SESSION_H__
#define __SOCK_SESSION_H__
#include "win_sock.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPort;
	class SocketSession;

	class SessionReceiver
	{
	public:
		SessionReceiver(SocketSession* session);
		virtual ~SessionReceiver();

		int BeginReceive();
		void OnReceive(DWORD dwError, DWORD dwBytesTransferred);

		SessionReceiver(const SessionReceiver&) = delete;
		SessionReceiver operator=(const SessionReceiver&) = delete;

	private:
		SocketSession* session_;
		OverlappedContext overlapped_context_;

		uint8_t buffer_[kSocketBufferSize];
	};

	class SocketSession
		: public WinSock
		, public OverlappedEventInterface
	{
	public:
		SocketSession();
		virtual ~SocketSession() {}

		void OnAccept(IoCompletionPort* iocp);
		void OnReceivePacket(const uint8_t* buffer, uint32_t received) {}

		virtual void OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) override;

	private:
		IoCompletionPort* iocp_{ nullptr };
		SessionReceiver receiver_;
	};
}

#endif
