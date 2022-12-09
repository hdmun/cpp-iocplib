#ifndef __WINSOCKLIB_CLIENT_SOCKET_H__
#define __WINSOCKLIB_CLIENT_SOCKET_H__

#include "winsocklib/win_sock.h"
#include "iocplib/packet.h"
#include "iocplib/socket_buffer.h"
#include "socket_io_thread.h"

namespace winsocklib {
	template<typename _Session>
	class ClientSocket
		: public std::enable_shared_from_this<ClientSocket<_Session> >
	{
	protected:
		ClientSocket()
			: sending_size_(0U)
		{
		}

		virtual ~ClientSocket() {}

	public:
		bool Connect(const SOCKADDR_IN& addr_in)
		{
			socket_.Create(AF_INET, SOCK_STREAM, 0);
			socket_.SetNonBlocking(true);

			io_thread_.RequestRegisterSocket(this->weak_from_this(), FD_READ | FD_WRITE | FD_CLOSE);

			return socket_.Connect(reinterpret_cast<const SOCKADDR*>(&addr_in));
		}

		void Close()
		{
			io_thread_.RequestUnregisterSocket(this->weak_from_this());
			sending_size_ = 0U;
			socket_.Close();
		}

		// for SocketIoThreadCallback
		const SOCKET GetSocketHandle() { return socket_.handle(); }
		void OnSocketRead(int error)
		{
			if (error > 0) {
				return;
			}

			while (true) {
				uint8_t* buffer = recv_buffer_ + recv_offset_;
				int buffer_len = winsocklib::kSocketBufferSize - recv_offset_;
				int received = socket_.Recv(reinterpret_cast<char*>(buffer), buffer_len, 0);
				if (received <= 0) {
					break;
				}

				uint32_t remain_length = received;
				do {
					remain_length = packet_receiver_.OnReceive(buffer, received);
					if (packet_receiver_.IsComplete()) {
						// on packet
						reinterpret_cast<_Session*>(this)->OnPacket(packet_receiver_.buffer());
						packet_receiver_.Clear();
					}
				} while (remain_length > 0);
			}
		}

		void OnSocketWrite(int error)
		{
			if (error > 0) {
				return;
			}

			TrySendPacket();
		}

		void OnSocketClose(int error)
		{
			Close();
		}

		void SendPacket(uint8_t* data, uint32_t size)
		{
			send_buffers_.push_back(iocplib::SocketBuffer::Allocate(data, size));

			TrySendPacket();
		}

	private:
		void TrySendPacket()
		{
			while (!send_buffers_.empty()) {
				const auto& front = send_buffers_.front();

				int send_len = front->GetBufferSize() - sending_size_;
				int len = socket_.Send(front->buffer() + sending_size_, send_len);
				if (len == 0) {
					break;
				}

				sending_size_ += len;
				if (sending_size_ < front->GetBufferSize()) {
					continue;
				}

				send_buffers_.pop_front();
				sending_size_ = 0;
			}
		}

	private:
		winsocklib::WinSock socket_;
		SocketIoThread<ClientSocket<_Session> > io_thread_;

		// recv
		uint8_t recv_buffer_[winsocklib::kSocketBufferSize]{ 0, };
		uint32_t recv_offset_{ 0 };
		iocplib::PacketReceiver packet_receiver_;

		// send buffer
		std::mutex send_lock_;
		std::list<std::unique_ptr<iocplib::SocketBuffer> > send_buffers_;
		uint32_t sending_size_;
	};
}
#endif
