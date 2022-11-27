#ifndef __WINSOCKLIB_CLIENT_SOCKET_CPP__
#define __WINSOCKLIB_CLIENT_SOCKET_CPP__

namespace winsocklib {
	template<typename _Session>
	ClientSocket<_Session>::ClientSocket()
		: sending_size_(0U)
	{
	}

	template<typename _Session>
	bool ClientSocket<_Session>::Connect(const SOCKADDR_IN& addr_in)
	{
		socket_.Create(AF_INET, SOCK_STREAM, 0);
		socket_.SetNonBlocking(true);

		io_thread_.RequestRegisterSocket(this->weak_from_this(), FD_READ | FD_WRITE | FD_CLOSE);

		return socket_.Connect(reinterpret_cast<const SOCKADDR*>(&addr_in));
	}

	template<typename _Session>
	void ClientSocket<_Session>::Close()
	{
		io_thread_.RequestUnregisterSocket(this->weak_from_this());
		sending_size_ = 0U;
		socket_.Close();
	}

	template<typename _Session>
	void ClientSocket<_Session>::OnSocketRead(int error)
	{
		if (error > 0) {
			return;
		}

		while (true) {
			uint8_t* buffer = recv_buffer_ + recv_offset_;
			int buffer_len = iocplib::kSocketBufferSize - recv_offset_;
			int received = socket_.Recv(reinterpret_cast<char*>(buffer), buffer_len, 0);
			if (received < 0) {
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

	template<typename _Socket>
	void ClientSocket<_Socket>::OnSocketWrite(int error)
	{
		if (error > 0) {
			return;
		}

		TrySendPacket();
	}

	template<typename _Socket>
	void ClientSocket<_Socket>::OnSocketClose(int error)
	{
		Close();
	}

	template<typename _Socket>
	void ClientSocket<_Socket>::SendPacket(uint8_t* data, uint32_t size)
	{
		send_buffers_.push_back(iocplib::SocketBuffer::Allocate(data, size));

		TrySendPacket();
	}

	template<typename _Socket>
	void ClientSocket<_Socket>::TrySendPacket()
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
}
#endif