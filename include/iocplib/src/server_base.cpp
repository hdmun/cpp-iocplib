#ifndef __SERVER_BASE_CPP__
#define __SERVER_BASE_CPP__

namespace iocplib {
	template <typename _Session>
	ServerBase<_Session>::ServerBase()
		: acceptor_(std::make_unique<Acceptor<_Session> >())
	{
	}

	template <typename _Session>
	bool ServerBase<_Session>::Open(uint16_t port, uint32_t io_thread)
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

	template <typename _Session>
	void ServerBase<_Session>::Close()
	{
		if (acceptor_) {
			acceptor_->Close();
		}
	}
}

#endif