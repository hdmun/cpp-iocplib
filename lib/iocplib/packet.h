#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

namespace iocplib {
	class PacketReceiver
	{
		enum {
			kDefaultBufferSize = 512,
			kPacketLengthSize = sizeof(uint32_t),
		};
	public:
		PacketReceiver()
		{
			buffer_.resize(kDefaultBufferSize);
		}

		virtual ~PacketReceiver() {}

		void Clear()
		{
			length_ = 0;
			offset_ = 0;
		}

		uint32_t OnReceive(uint8_t* buffer, uint32_t received)
		{

			uint32_t appended = 0;

			if (offset_ < kPacketLengthSize) {
				// 패킷 맨 앞 4byte는 길이 정보를 포함하고 있다.
				uint32_t append_size = std::min(received, kPacketLengthSize - offset_);
				Append(buffer, append_size);

				if (offset_ >= kPacketLengthSize) {
					length_ = *reinterpret_cast<uint32_t*>(buffer_.data());
					if (length_ > kDefaultBufferSize) {
						buffer_.reserve(length_);
					}
				}

				received -= append_size;
				if (received == 0U) {
					return 0U;
				}

				// 처리해야할 데이터가 남았다.
				buffer += append_size;
				appended += append_size;
			}

			uint32_t append_size = std::min(received, length_ + kPacketLengthSize - offset_);
			Append(buffer, append_size);
			return received - append_size;
		}

		bool IsComplete() const
		{
			return length_ + kPacketLengthSize == offset_;
		}

		const uint8_t* buffer() const
		{
			return buffer_.data() + kPacketLengthSize;
		}

	private:
		void Append(uint8_t* buffer, uint32_t append)
		{
			auto append_size = buffer_.size() + append;
			if (append_size > buffer_.size()) {
				buffer_.resize(append_size);
			}

			uint8_t* current_data = buffer_.data() + offset_;
			std::copy(buffer, buffer + append, current_data);
			offset_ += append;
		}

	private:
		uint32_t length_{ 0U };

		uint32_t offset_{ 0U };
		std::vector<uint8_t> buffer_;
	};
}

#endif
