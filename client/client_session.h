#ifndef __CLIENT_SESSION_H__
#define __CLIENT_SESSION_H__
#include "winsocklib/client_socket.h"

namespace message {
	namespace server {
		struct LoginResult;
		struct LogoutResult;
		struct ConnectServerResult;
	}
}

class ClientSession
	: public winsocklib::ClientSocket<ClientSession>
{
public:
	ClientSession();
	virtual ~ClientSession() {}

	void OnPacket(const uint8_t* buffer);

	void SendLoginRequest(std::string id, std::string password);
	void _LoginResult(const message::server::LoginResult* result);

	void ConnectServerRequest(int16_t server_id);
	void _ConnectServerResult(const message::server::ConnectServerResult* result);

	void SendLogoutRequest();
	void _LogoutResult(const message::server::LogoutResult* result);
};

#endif