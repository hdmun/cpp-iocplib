#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include "../winsocklib/win_sock.h"
#include "io_completion_port.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPort;
	struct OverlappedEventInterface;
	struct OverlappedContext;

	template<typename _Socket>
	class AcceptSocket
		: public OverlappedContext
	{
	public:
		AcceptSocket()
		{
			::ZeroMemory(accept_addr_buffer_, sizeof(accept_addr_buffer_));
		}

		virtual ~AcceptSocket() {}

		void PostAccept(SOCKET listen_socket)
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

		LPSOCKADDR GetRemoteAddr()
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

		void OnAccept(SOCKET listen_socket)
		{
			socket_->SetAccept(listen_socket);
		}

		std::shared_ptr<_Socket> socket() const { return socket_;  }

	private:
		enum {
			kAddressBufferSize = 16 + sizeof(SOCKADDR_IN),
		};

		TCHAR accept_addr_buffer_[kAddressBufferSize * 2];
		std::shared_ptr<_Socket> socket_;
	};

	template<typename _Socket>
	class Acceptor
		: public OverlappedEventInterface
		, public std::enable_shared_from_this<Acceptor<_Socket> >
	{
	public:
		Acceptor()
			: io_completion_port_(std::make_unique<IoCompletionPort>())
		{
		}

		virtual ~Acceptor() {}

		bool Open(uint32_t io_thread, const SOCKADDR_IN* pAddr, int backlog = SOMAXCONN, int accepts = 1)
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
				socket->callback = this->shared_from_this();
				socket->data.obj = socket.get();
				socket->PostAccept(listen_socket_.handle());
				accept_sockets_.push_back(std::move(socket));
			}

			opened_ = true;
			return true;
		}

		void Close()
		{
			if (opened_) {
				io_completion_port_->Detach();
			}

			io_completion_port_->Close();
			io_completion_port_ = nullptr;

			listen_socket_.Close();
			opened_ = false;
		}

		virtual void OnCompleteOverlappedIO(const OverlappedContext::Data& data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey)
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

		void OnAccept(DWORD dwError, std::shared_ptr<_Socket> socket, const SOCKADDR_IN* pAddr)
		{
			if (dwError != 0) {
				// accept error
				return;
			}

			{
				socket->OnAccept(io_completion_port_.get());
				std::lock_guard<std::mutex> _lock(sockets_lock_);
				sockets_.push_back(socket);
			}
		}

	private:
		std::unique_ptr< IoCompletionPort > io_completion_port_;
		winsocklib::WinSock listen_socket_;
		bool opened_{ false };

		std::vector< std::unique_ptr<AcceptSocket<_Socket> > > accept_sockets_;

		std::mutex sockets_lock_;
		std::list< std::shared_ptr< _Socket > > sockets_;  // connections
	};
}
#endif