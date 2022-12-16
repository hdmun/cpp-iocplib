#ifndef __SERVER_BASE_H__
#define __SERVER_BASE_H__
#include "acceptor.h"
#include "socket_session.h"

namespace iocplib {
	template <typename _Session>
	class ServerBase
	{
	public:
		ServerBase()
			: acceptor_(std::make_shared<Acceptor<_Session> >())
		{
		}

		virtual ~ServerBase() {}

		bool Open(uint16_t port, uint32_t io_thread)
		{
			SOCKADDR_IN addr_in = { 0, };
			addr_in.sin_family = AF_INET;
			addr_in.sin_port = ::htons(port);
			addr_in.sin_addr.s_addr = ::htonl(INADDR_ANY);

			if (!acceptor_->Open(io_thread, &addr_in)) {
				std::cout << "failed open accepter" << std::endl;
				return false;
			}

			return true;
		}

		void Close()
		{
			if (acceptor_) {
				acceptor_->Close();
			}
		}

		ServerBase(const ServerBase&) = delete;
		ServerBase operator=(const ServerBase&) = delete;

	private:
		std::shared_ptr<Acceptor<_Session> > acceptor_;
	};
}
#endif
