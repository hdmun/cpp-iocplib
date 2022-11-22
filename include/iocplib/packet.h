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
		PacketReceiver();
		virtual ~PacketReceiver() {}

		void Clear();
		uint32_t OnReceive(uint8_t* buffer, uint32_t received);

		bool IsComplete() const;
		const uint8_t* buffer() const;

	private:
		void Append(uint8_t* buffer, uint32_t append);

	private:
		uint32_t length_{ 0U };

		uint32_t offset_{ 0U };
		std::vector<uint8_t> buffer_;
	};
}

#endif
