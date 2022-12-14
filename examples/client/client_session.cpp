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

void ClientSession::OnPacket(const uint8_t* buffer, uint32_t length)
{
	using msg = message::server::PacketType;

	flatbuffers::Verifier verifier(buffer, length);
	if (!message::server::VerifyPacketBuffer(verifier)) {
		// invalid packet
		return;
	}

	const auto* packet = message::server::GetPacket(buffer);
	switch (packet->type_type())
	{
		PACKET_HANDLER(msg::PacketType_LoginResult, _LoginResult);
		PACKET_HANDLER(msg::PacketType_ConnectServerResult, _ConnectServerResult);
		PACKET_HANDLER(msg::PacketType_LogoutResult, _LogoutResult);
	default:
		break;
	}
}


void ClientSession::SendLoginRequest(std::string id, std::string password)
{
	flatbuffers::FlatBufferBuilder builder;
	auto request = message::client::CreateLoginRequestDirect(builder, id.c_str(), password.c_str());
	auto packet = message::client::CreatePacket(builder, message::client::PacketType_LoginRequest, request.Union());
	builder.Finish(packet);

	SendPacket(builder.GetBufferPointer(), builder.GetSize());
}

void ClientSession::_LoginResult(const message::server::LoginResult* result)
{
	int32_t error = result->error();

	std::cout << "_LoginResult: " << error << std::endl;

	if (error == 0) {
		ConnectServerRequest(1);
	}
}

void ClientSession::ConnectServerRequest(int16_t server_id)
{
	flatbuffers::FlatBufferBuilder builder;
	auto request = message::client::CreateConnectServerRequest(builder, server_id);
	auto packet = message::client::CreatePacket(builder, message::client::PacketType_ConnectServerRequest, request.Union());
	builder.Finish(packet);

	SendPacket(builder.GetBufferPointer(), builder.GetSize());
}

void ClientSession::_ConnectServerResult(const message::server::ConnectServerResult* result)
{
	std::cout << "_ConnectServerResult: " << result->error() << std::endl;

	SendLogoutRequest();
}

void ClientSession::SendLogoutRequest()
{
	flatbuffers::FlatBufferBuilder builder;
	auto request = message::client::CreateLogoutRequest(builder);
	auto packet = message::client::CreatePacket(builder, message::client::PacketType_LogoutRequest, request.Union());
	builder.Finish(packet);

	SendPacket(builder.GetBufferPointer(), builder.GetSize());
}

void ClientSession::_LogoutResult(const message::server::LogoutResult* result)
{
	std::cout << "_LogoutResult: " << std::endl;
}
