#ifndef __WINSOCKLIB_CLIENT_SOCKET_H__
#define __WINSOCKLIB_CLIENT_SOCKET_H__

#include "iocplib/win_sock.h"
#include "iocplib/packet.h"
#include "iocplib/socket_buffer.h"
#include "socket_io_thread.h"

namespace winsocklib {
	template<typename _Session>
	class ClientSocket
		: public std::enable_shared_from_this<ClientSocket<_Session> >
	{
	protected:
		ClientSocket();
		virtual ~ClientSocket() {}

	public:
		bool Connect(const SOCKADDR_IN& addr_in);
		void Close();

		// for SocketIoThreadCallback
		const SOCKET GetSocketHandle() { return socket_.handle(); }
		void OnSocketRead(int error);
		void OnSocketWrite(int error);
		void OnSocketClose(int error);

		void SendPacket(uint8_t* data, uint32_t size);

	private:
		void TrySendPacket();

	private:
		iocplib::WinSock socket_;
		SocketIoThread<ClientSocket<_Session> > io_thread_;

		// recv
		uint8_t recv_buffer_[iocplib::kSocketBufferSize]{ 0, };
		uint32_t recv_offset_{ 0 };
		iocplib::PacketReceiver packet_receiver_;

		// send buffer
		std::mutex send_lock_;
		std::list<std::unique_ptr<iocplib::SocketBuffer> > send_buffers_;
		uint32_t sending_size_;
	};
}

#include "src/client_socket.cpp"
#endif

