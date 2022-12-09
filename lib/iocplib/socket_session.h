#ifndef __SOCK_SESSION_H__
#define __SOCK_SESSION_H__
#include "../winsocklib/win_sock.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPort;
	class SocketBuffer;
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

		uint8_t buffer_[winsocklib::kSocketBufferSize];
	};

	class SessionSender
	{
	public:
		SessionSender(SocketSession* session);
		virtual ~SessionSender();

		SessionSender(const SessionSender&) = delete;
		SessionSender operator=(const SessionSender&) = delete;

		void SendAsync(uint8_t* data, uint32_t size);

		int BeginSend();
		void OnSend(DWORD dwError, DWORD dwBytesTransferred);

	private:
		void PostCompletionPortSignal(HANDLE iocp_handle);

	private:
		SocketSession* session_;
		OverlappedContext overlapped_context_;

		bool post_send_signal{ false };
		std::queue<std::unique_ptr<SocketBuffer> > send_queue_;

		std::list<std::unique_ptr<SocketBuffer> > sending_packets_;
		std::uint32_t sending_offset_{ 0 };
	};

	class SocketSession
		: public winsocklib::WinSock
		, public OverlappedEventInterface
	{
	protected:
		SocketSession();
		virtual ~SocketSession() {}

	public:
		HANDLE GetIocpHandle() const;

		void OnAccept(IoCompletionPort* iocp);
		virtual void OnReceivePacket(uint8_t* buffer, uint32_t received) {}

		virtual void OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) override;

		void SendAsync(uint8_t* data, uint32_t size) {
			sender_.SendAsync(data, size);
		}

	private:
		IoCompletionPort* iocp_{ nullptr };
		SessionReceiver receiver_;
		SessionSender sender_;
	};
}

#endif
