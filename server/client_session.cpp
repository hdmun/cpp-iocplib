#include "pch.h"
#include "client_session.h"

ClientSession::ClientSession()
{
}

void ClientSession::OnReceivePacket(uint8_t* buffer, uint32_t received)
{
	uint32_t remain_length = received;
	do {
		remain_length = packet_receiver_.OnReceive(buffer, remain_length);
		if (packet_receiver_.IsComplete()) {
			// deserialize
			packet_receiver_.Clear();
		}
	} while (remain_length > 0);
}
