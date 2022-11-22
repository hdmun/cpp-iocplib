#ifndef __CLIENT_SESSION_H__
#define __CLIENT_SESSION_H__
#include "iocplib/socket_session.h"
#include "iocplib/packet.h"

class ClientSession
	: public iocplib::SocketSession
{
public:
	ClientSession();
	virtual ~ClientSession() {}

	virtual void OnReceivePacket(uint8_t* buffer, uint32_t received);

private:
	iocplib::PacketReceiver packet_receiver_;
};

#endif