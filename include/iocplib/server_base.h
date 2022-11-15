#ifndef __SERVER_BASE_H__
#define __SERVER_BASE_H__
#include "acceptor.h"
#include "socket_session.h"

namespace iocplib {
	class ServerBase
	{
	public:
		ServerBase();
		virtual ~ServerBase() {}

		bool Open(uint16_t port, uint32_t io_thread);
		void Close();

		ServerBase(const ServerBase&) = delete;
		ServerBase operator=(const ServerBase&) = delete;

	private:
		std::unique_ptr<Acceptor<SocketSession> > acceptor_;
	};
}

#endif
