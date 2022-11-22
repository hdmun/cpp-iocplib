#include "pch.h"
#include "../packet.h"

namespace iocplib {
	PacketReceiver::PacketReceiver()
	{
		buffer_.resize(kDefaultBufferSize);
	}

	void PacketReceiver::Clear()
	{
		length_ = 0;
		offset_ = 0;
	}

	uint32_t PacketReceiver::OnReceive(uint8_t* buffer, uint32_t received)
	{
		uint32_t appended = 0;

		if (offset_ < kPacketLengthSize) {
			// ��Ŷ �� �� 4byte�� ���� ������ �����ϰ� �ִ�.
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

			// ó���ؾ��� �����Ͱ� ���Ҵ�.
			buffer += append_size;
			appended += append_size;
		}

		uint32_t append_size = std::min(received, length_ + kPacketLengthSize - offset_);
		Append(buffer, append_size);
		return received - append_size;
	}

	bool PacketReceiver::IsComplete() const
	{
		return length_ + kPacketLengthSize == offset_;
	}

	const uint8_t* PacketReceiver::buffer() const
	{
		return buffer_.data();
	}

	void PacketReceiver::Append(uint8_t* buffer, uint32_t append)
	{
		auto append_size = buffer_.size() + append;
		if (append_size > buffer_.size()) {
			buffer_.resize(append_size);
		}

		uint8_t* current_data = buffer_.data() + offset_;
		std::copy(buffer, buffer + append, current_data);
		offset_ += append;
	}
}
