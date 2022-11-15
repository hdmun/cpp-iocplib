#include "pch.h"
#include "../server_base.h"

namespace iocplib {
	ServerBase::ServerBase()
		: acceptor_(std::make_unique<Acceptor<SocketSession> >())
	{
	}

	bool ServerBase::Open(uint16_t port, uint32_t io_thread)
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

	void ServerBase::Close()
	{
		if (acceptor_) {
			acceptor_->Close();
		}
	}
}
