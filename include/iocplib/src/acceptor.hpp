#include "../acceptor.h"
#include "../io_completion_port.h"

namespace iocplib {
	template<typename _Socket>
	AcceptSocket<_Socket>::AcceptSocket()
	{
		::ZeroMemory(accept_addr_buffer_, sizeof(accept_addr_buffer_));
	}

	template<typename _Socket>
	void AcceptSocket<_Socket>::PostAccept(SOCKET listen_socket)
	{
		socket_ = std::make_shared<_Socket>();
		socket_->Create(AF_INET, SOCK_STREAM, 0);

		// https://learn.microsoft.com/en-us/windows/win32/api/mswsock/nf-mswsock-acceptex
		DWORD dwBytesReceived = 0;
		BOOL bRet = ::AcceptEx(
			listen_socket,
			socket_->handle(),
			accept_addr_buffer_,
			0, // dwReceiveDataLength
			kAddressBufferSize, // dwLocalAddressLength
			kAddressBufferSize, // dwRemoteAddressLength
			&dwBytesReceived,
			this // lpOverlapped
		);

		int err = ::WSAGetLastError();
		if (!bRet && err != ERROR_IO_PENDING) {
			// throw exception err;
		}
	}

	template<typename _Socket>
	LPSOCKADDR AcceptSocket<_Socket>::GetRemoteAddr()
	{
		LPSOCKADDR pLocalAddr = nullptr;
		INT localLen = 0;
		LPSOCKADDR pRemoteAddr = nullptr;
		INT remoteLen = 0;
		::GetAcceptExSockaddrs(
			accept_addr_buffer_,
			0, // dwReceiveDataLength
			kAddressBufferSize, // dwLocalAddressLength
			kAddressBufferSize, // dwRemoteAddressLength
			&pLocalAddr, &localLen, &pRemoteAddr, &remoteLen);
		return pRemoteAddr;
	}

	template<typename _Socket>
	void AcceptSocket<_Socket>::OnAccept(SOCKET listen_socket)
	{
		socket_->SetAccept(listen_socket);
	}

	template<typename _Socket>
	Acceptor<_Socket>::Acceptor()
		: io_completion_port_(std::make_unique<IoCompletionPort>())
	{

	}

	template<typename _Socket>
	Acceptor<_Socket>::~Acceptor()
	{
	}

	template<typename _Socket>
	bool Acceptor<_Socket>::Open(uint32_t io_thread, const SOCKADDR_IN* pAddr, int backlog /*= SOMAXCONN*/, int accepts /*= 1 */)
	{
		if (!io_completion_port_->Create(io_thread)) {
			// log
			return false;
		}

		listen_socket_.Create(AF_INET, SOCK_STREAM, 0);
		listen_socket_.Bind(reinterpret_cast<SOCKADDR*>(const_cast<SOCKADDR_IN*> (pAddr)), sizeof(*pAddr));
		listen_socket_.Listen(backlog);

		io_completion_port_->Attach(reinterpret_cast<HANDLE>(listen_socket_.handle()));

		accept_sockets_.reserve(accepts);
		for (int i = 0; i < accepts; i++) {
			auto socket = std::make_unique<AcceptSocket<_Socket> >();
			socket->callback = this;
			socket->data.obj = socket.get();
			socket->PostAccept(listen_socket_.handle());
			accept_sockets_.push_back(std::move(socket));
		}

		return true;
	}

	template<typename _Socket>
	void Acceptor<_Socket>::Close()
	{
		io_completion_port_->Detach();
		io_completion_port_->Close();
		io_completion_port_ = nullptr;

		listen_socket_.Close();
	}

	template<typename _Socket>
	void Acceptor<_Socket>::OnComplete(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
	{
		AcceptSocket<_Socket>* accept_socket = reinterpret_cast<AcceptSocket<_Socket>*>(data.obj);

		try {
			if (dwError == 0) {
				LPSOCKADDR pAddrRemote = accept_socket->GetRemoteAddr();
				accept_socket->OnAccept(listen_socket_.handle());

				OnAccept(0, accept_socket->socket(), reinterpret_cast<SOCKADDR_IN*>(pAddrRemote));
			}
		}
		catch (...)
		{
		}

		accept_socket->PostAccept(listen_socket_.handle());
	}

	template<typename _Socket>
	void Acceptor<_Socket>::OnAccept(DWORD dwError, std::shared_ptr<_Socket> socket, const SOCKADDR_IN* pAddr)
	{
		if (dwError != 0) {
			// accept error
			return;
		}

		{
			std::lock_guard<std::mutex> _lock(sockets_lock_);
			sockets_.push_back(socket);
		}
	}
}