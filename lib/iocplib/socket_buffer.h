#ifndef __SOCKET_BUFFER_H__
#define __SOCKET_BUFFER_H__

namespace iocplib {
	class SocketBuffer {
	public:
		SocketBuffer(uint8_t* data, uint32_t size);
		SocketBuffer(std::vector<uint8_t>&& buffer);
		SocketBuffer(SocketBuffer&& rhs);

		virtual ~SocketBuffer() {}

		static std::unique_ptr<SocketBuffer> Allocate(uint8_t* data, uint32_t size);
		std::unique_ptr<SocketBuffer> Slice(int start, int size);

		uint8_t* buffer() {
			return buffer_.data();
		}

		uint32_t GetBufferSize() const {
			return static_cast<uint32_t>(buffer_.size());
		}

	private:
		std::vector<uint8_t> buffer_;
	};
}

#endif
