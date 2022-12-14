#include "pch.h"
#include "client_session.h"
#include "message/client_message_generated.h"
#include "message/server_message_generated.h"

#define PACKET_HANDLER(_TYPE, _HANDLER) \
	case _TYPE: \
		_HANDLER(packet->type_as##_HANDLER()); \
		break; \


ClientSession::ClientSession()
{
}

void ClientSession::OnReceivePacket(uint8_t* buffer, uint32_t received)
{
	uint32_t remain_length = received;
	do {
		remain_length = packet_receiver_.OnReceive(buffer, remain_length);
		if (packet_receiver_.IsComplete()) {
			const uint8_t* packet_buffer = packet_receiver_.buffer();
			flatbuffers::Verifier verifier(packet_buffer, packet_receiver_.length());
			if (!message::client::VerifyPacketBuffer(verifier)) {
				// close socket
				break;
			}

			const auto* packet = message::client::GetPacket(packet_buffer);
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
	std::cout << "_LoginRequest: ";
	std::cout << request->id()->c_str() << ", ";
	std::cout << request->password()->c_str() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto result = message::server::CreateLoginResult(builder, 0);
	auto packet = message::server::CreatePacket(builder, message::server::PacketType_LoginResult, result.Union());
	builder.Finish(packet);

	SendAsync(builder.GetBufferPointer(), builder.GetSize());
}

void ClientSession::_LogoutRequest(const message::client::LogoutRequest* request)
{
	std::cout << "_LogoutRequest: " << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto result = message::server::CreateLogoutResult(builder);
	auto packet = message::server::CreatePacket(builder, message::server::PacketType_LogoutResult, result.Union());
	builder.Finish(packet);

	SendAsync(builder.GetBufferPointer(), builder.GetSize());
}

void ClientSession::_ConnectServerRequest(const message::client::ConnectServerRequest* request)
{
	std::cout << "_ConnectServerRequest: ";
	std::cout << request->server_id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto result = message::server::CreateConnectServerResult(builder);
	auto packet = message::server::CreatePacket(builder, message::server::PacketType_ConnectServerResult, result.Union());
	builder.Finish(packet);

	SendAsync(builder.GetBufferPointer(), builder.GetSize());
}
