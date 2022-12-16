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
		SessionReceiver();
		virtual ~SessionReceiver();

		SessionReceiver(const SessionReceiver&) = delete;
		SessionReceiver operator=(const SessionReceiver&) = delete;

		void OnAccept(std::weak_ptr<SocketSession> session);

		int32_t BeginReceive();
		void OnReceive(DWORD dwError, DWORD dwBytesTransferred);

	private:
		int32_t OnError(DWORD dwError);

	private:
		std::weak_ptr<SocketSession> session_;

		std::recursive_mutex lock_;
		OverlappedContext overlapped_context_;
		bool zero_byet_recv_{ false };
		uint32_t recv_error_{ 0U };

		uint8_t buffer_[winsocklib::kSocketBufferSize];
	};

	class SessionSender
	{
	public:
		SessionSender();
		virtual ~SessionSender();

		SessionSender(const SessionSender&) = delete;
		SessionSender operator=(const SessionSender&) = delete;

		void OnAccept(std::weak_ptr<SocketSession> session);

		void SendAsync(uint8_t* data, uint32_t size);

		void OnSend(DWORD dwError, DWORD dwBytesTransferred);

	private:
		int32_t BeginSend();
		void PostCompletionPortSignal(HANDLE iocp_handle);

	private:
		std::weak_ptr<SocketSession> session_;

		std::recursive_mutex lock_;
		OverlappedContext overlapped_context_;

		bool post_send_signal{ false };
		std::queue<std::unique_ptr<SocketBuffer> > send_queue_;

		std::list<std::unique_ptr<SocketBuffer> > sending_packets_;
		std::uint32_t sending_offset_{ 0 };

		uint32_t send_error_{ 0U };
	};

	class SocketSession
		: public winsocklib::WinSock
		, public OverlappedEventInterface
		, public std::enable_shared_from_this<SocketSession>
	{
	protected:
		SocketSession();
		virtual ~SocketSession() {}

	public:
		HANDLE GetIocpHandle() const;

		void OnAccept(IoCompletionPort* iocp);
		virtual void OnReceivePacket(uint8_t* buffer, uint32_t received) {}

		virtual void OnCompleteOverlappedIO(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey) override;

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
