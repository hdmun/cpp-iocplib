#ifndef __WIN_SOCK_H__
#define __WIN_SOCK_H__

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

namespace winsocklib {

	enum {
		kSocketBufferSize = 1460,
	};

	class WinSock
	{
	public:
		WinSock()
			: handle_(INVALID_SOCKET)
		{
		};
		virtual ~WinSock() {}

		WinSock(const WinSock&) = delete;
		WinSock operator=(const WinSock&) = delete;

		void Create( int af, int type, int protocol )
		{
			handle_ = ::socket(af, type, protocol);
			if (handle_ == INVALID_SOCKET) {
				std::string msg = "failed create socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		void Bind(const SOCKADDR* pAddr, int len)
		{
			int bind_ = ::bind(handle_, pAddr, len);
			if (bind_ == SOCKET_ERROR) {
				std::string msg = "failed bind socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		void Listen(int backlog = SOMAXCONN)
		{
			int bind_ = ::listen(handle_, backlog);
			if (bind_ == SOCKET_ERROR) {
				std::string msg = "failed listen socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		bool Connect(const SOCKADDR* pAddr)
		{
			int ret = ::connect(handle_, pAddr, sizeof(*pAddr));
			if (ret != SOCKET_ERROR || ::WSAGetLastError() == WSAEWOULDBLOCK) {
				return true;
			}
			return false;
		}

		void Close()
		{
			if (handle_ != INVALID_SOCKET) {
				int ret = ::closesocket(handle_);
				if (ret == SOCKET_ERROR) {
					// hmm...
				}

				handle_ = INVALID_SOCKET;
			}
		}

		int Send(void* data, int len, int flags = 0)
		{
			int sended = ::send(handle_, reinterpret_cast<char*>(data), len, flags);
			if (sended == SOCKET_ERROR) {
				int err = ::WSAGetLastError();
				if (err == WSAEWOULDBLOCK) {
					return 0;
				}

				// throw exception
			}

			return sended;
		}

		int Recv(char* buffer, int len, int flag)
		{
			int received = ::recv(handle_, buffer, len, flag);
			if (received == SOCKET_ERROR) {
				int err = ::WSAGetLastError();
				if (err == WSAEWOULDBLOCK) {
					return -1;
				}

				// throw exception
			}

			return received;
		}

		void SetAccept(SOCKET listen_socket)
		{
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
				reinterpret_cast<const char*>(&listen_socket), sizeof(listen_socket)
			);
			if (ret == SOCKET_ERROR) {
				std::string msg = "failed update accept socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		void SetNonBlocking(bool nonblocking)
		{
			u_long b = nonblocking ? 1UL : 0UL;
			int ret = ::ioctlsocket(handle_, FIONBIO, &b);
			if (ret == SOCKET_ERROR) {
				std::string msg = "failed set non blocking socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		void SetRecvBufferSize(uint32_t size)
		{
			int n = size;
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_RCVBUF,
				reinterpret_cast<const char*>(&n), sizeof(n));
			if (ret == SOCKET_ERROR) {
				std::string msg = "failed set receive buffer size socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		void SetSendBufferSize( uint32_t size )
		{
			int n = size;
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_SNDBUF,
				reinterpret_cast<const char*>(&n), sizeof(n));
			if (ret == SOCKET_ERROR) {
				std::string msg = "failed set send buffer size socket, " + std::to_string(::WSAGetLastError());
				throw std::exception(msg.c_str());
			}
		}

		SOCKET handle() const { return handle_; }

	protected:
		SOCKET handle_;
	};


	struct WinSockInitializer
	{
		WinSockInitializer()
		{
			WSADATA wsadata = { 0, };
			int ret = ::WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
			if ( ret != 0 || LOBYTE( wsadata.wVersion ) != 2 ) {
				std::cout << "failed WSAStartup" << std::endl;
			}
		}

		~WinSockInitializer()
		{
			int ret = ::WSACleanup();
			if ( ret != 0 ) {
				std::cout << "failed WSACleanup" << std::endl;
			}
		}
	};
}

#endif
