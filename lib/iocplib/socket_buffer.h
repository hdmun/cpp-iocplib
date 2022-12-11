#ifndef __SOCKET_BUFFER_H__
#define __SOCKET_BUFFER_H__

namespace iocplib {
	class SocketBuffer {
	public:
		SocketBuffer(uint8_t* data, uint32_t size)
		{
			constexpr uint32_t header_len = 4;
			buffer_.resize(size + header_len);
			std::copy(data, data + size, buffer_.begin() + header_len);
			*reinterpret_cast<uint32_t*>(buffer_.data()) = size;
		}

		SocketBuffer(std::vector<uint8_t>&& buffer)
			: buffer_(std::move(buffer))
		{
		}

		SocketBuffer(SocketBuffer&& rhs)
			: buffer_(std::move(rhs.buffer_))
		{
		}

		virtual ~SocketBuffer() {}

		static std::unique_ptr<SocketBuffer> Allocate(uint8_t* data, uint32_t size)
		{
			return std::make_unique<SocketBuffer>(data, size);
		}

		std::unique_ptr<SocketBuffer> Slice(int start, int size)
		{
			if (buffer_.size() <= start + size) {
				return nullptr;
			}

			const auto startIter = buffer_.begin() + start;
			return std::make_unique<SocketBuffer>(
				std::move(std::vector<uint8_t>(startIter, startIter + size))
				);
		}

		uint8_t* buffer()
		{
			return buffer_.data();
		}

		uint32_t GetBufferSize() const
		{
			return static_cast<uint32_t>(buffer_.size());
		}

	private:
		std::vector<uint8_t> buffer_;
	};
}

#endif
