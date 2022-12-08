#ifndef __CLIENT_SESSION_H__
#define __CLIENT_SESSION_H__
#include "iocplib/socket_session.h"
#include "iocplib/packet.h"

namespace message {
	namespace client {
		struct Packet;
		struct LoginRequest;
		struct LogoutRequest;
		struct ConnectServerRequest;
	}
}

class ClientSession
	: public iocplib::SocketSession
{
public:
	ClientSession();
	virtual ~ClientSession() {}

	virtual void OnReceivePacket(uint8_t* buffer, uint32_t received);

private:
	void OnPacket(const message::client::Packet* packet);

	void _LoginRequest(const message::client::LoginRequest* request);
	void _LogoutRequest(const message::client::LogoutRequest* request);
	void _ConnectServerRequest(const message::client::ConnectServerRequest* request);

private:
	iocplib::PacketReceiver packet_receiver_;
};

#endif