#include "pch.h"
#include "../acceptor.h"
#include "../io_completion_port.h"

namespace iocplib {
	AcceptSocket::AcceptSocket()
	{
		::ZeroMemory(accept_addr_buffer_, sizeof(accept_addr_buffer_));
	}

	void AcceptSocket::PostAccept(SOCKET listen_socket)
	{
		socket_ = std::make_shared<WinSock>();
		socket_->Create(AF_INET, SOCK_STREAM, 0);

		const DWORD dwLocalAddressLength = 16 + sizeof(SOCKADDR_IN);
		const DWORD dwRemoteAddressLength = 16 + sizeof(SOCKADDR_IN);

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

	LPSOCKADDR AcceptSocket::GetRemoteAddr()
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


	void AcceptSocket::OnAccept(SOCKET listen_socket)
	{
		socket_->SetAccept(listen_socket);
	}

	Acceptor::Acceptor()
		: io_completion_port_(std::make_unique<IoCompletionPort>())
	{

	}

	Acceptor::~Acceptor()
	{
	}

	bool Acceptor::Open(uint32_t io_thread, const SOCKADDR_IN* pAddr, int backlog /*= SOMAXCONN*/, int accepts /*= 1 */)
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
			auto socket = std::make_unique<AcceptSocket>();
			socket->callback = this;
			socket->data = socket.get();
			socket->PostAccept(listen_socket_.handle());
			accept_sockets_.push_back(std::move(socket));
		}

		return true;
	}

	void Acceptor::Close()
	{
		io_completion_port_->Detach();
		io_completion_port_ = nullptr;

		listen_socket_.Close();
	}

	void Acceptor::OnComplete(void* data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
	{
		AcceptSocket* accept_socket = reinterpret_cast<AcceptSocket*>(data);

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

	void Acceptor::OnAccept(DWORD dwError, std::shared_ptr<WinSock> socket, const SOCKADDR_IN* pAddr)
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