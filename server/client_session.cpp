#include "pch.h"
#include "client_session.h"
#include "message/client_message_generated.h"

#define PACKET_HANDLER(_TYPE, _HANDLER) \
	case _TYPE: _HANDLER(packet->type_as##_HANDLER())


ClientSession::ClientSession()
{
}

void ClientSession::OnReceivePacket(uint8_t* buffer, uint32_t received)
{
	uint32_t remain_length = received;
	do {
		remain_length = packet_receiver_.OnReceive(buffer, remain_length);
		
		if (packet_receiver_.IsComplete()) {
			const auto* packet = message::client::GetPacket(packet_receiver_.buffer());
			OnPacket(packet);

			packet_receiver_.Clear();
		}
	} while (remain_length > 0);
}

void ClientSession::OnPacket(const message::client::Packet* packet)
{
	using msg = message::client::PacketType;

	switch (packet->type_type())
	{
		PACKET_HANDLER(msg::PacketType_LoginRequest, _LoginRequest);
		PACKET_HANDLER(msg::PacketType_ConnectServerRequest, _ConnectServerRequest);
		PACKET_HANDLER(msg::PacketType_LogoutRequest, _LogoutRequest);
	default:
		break;
	}
}

void ClientSession::_LoginRequest(const message::client::LoginRequest* request)
{
	request->id();
	request->password();
}

void ClientSession::_LogoutRequest(const message::client::LogoutRequest* request)
{
	request;
}

void ClientSession::_ConnectServerRequest(const message::client::ConnectServerRequest* request)
{
	request->server_id();
}
