#ifndef __SERVER_BASE_H__
#define __SERVER_BASE_H__
#include "acceptor.h"
#include "socket_session.h"

namespace iocplib {
	template <typename _Session>
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
		std::unique_ptr<Acceptor<_Session> > acceptor_;
	};
}

#include "src/server_base.cpp"
#endif
