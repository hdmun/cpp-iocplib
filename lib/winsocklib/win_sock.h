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
				assert(false);
				throw WinSockException("failed to create socket", ::WSAGetLastError());
			}
		}

		void Bind(const SOCKADDR* pAddr, int len)
		{
			int bind_ = ::bind(handle_, pAddr, len);
			if (bind_ == SOCKET_ERROR) {
				assert(false);
				throw WinSockException("failed to bind socket", ::WSAGetLastError());
			}
		}

		void Listen(int backlog = SOMAXCONN)
		{
			int bind_ = ::listen(handle_, backlog);
			if (bind_ == SOCKET_ERROR) {
				assert(false);
				throw WinSockException("failed to listen socket", ::WSAGetLastError());
			}
		}

		bool Connect(const SOCKADDR* pAddr)
		{
			int ret = ::connect(handle_, pAddr, sizeof(*pAddr));
			if (ret != SOCKET_ERROR || ::WSAGetLastError() == WSAEWOULDBLOCK) {
				return true;
			}

			assert(false && "failed to connect socket");
			return false;
		}

		void Close()
		{
			if (handle_ != INVALID_SOCKET) {
				int ret = ::closesocket(handle_);
				if (ret == SOCKET_ERROR) {
					assert(false && "failed to close socket");
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

				throw WinSockException("failed to send socket", err);
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

				throw WinSockException("failed to recv socket", err);
			}

			return received;
		}

		void SetAccept(SOCKET listen_socket)
		{
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
				reinterpret_cast<const char*>(&listen_socket), sizeof(listen_socket)
			);
			if (ret == SOCKET_ERROR) {
				throw WinSockException("failed to update accept socket", ::WSAGetLastError());
			}
		}

		void SetNonBlocking(bool nonblocking)
		{
			u_long b = nonblocking ? 1UL : 0UL;
			int ret = ::ioctlsocket(handle_, FIONBIO, &b);
			if (ret == SOCKET_ERROR) {
				throw WinSockException("failed set non blocking socket", ::WSAGetLastError());
			}
		}

		void SetRecvBufferSize(uint32_t size)
		{
			int n = size;
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_RCVBUF,
				reinterpret_cast<const char*>(&n), sizeof(n));
			if (ret == SOCKET_ERROR) {
				throw WinSockException("failed to set receive buffer size socket", ::WSAGetLastError());
			}
		}

		void SetSendBufferSize( uint32_t size )
		{
			int n = size;
			int ret = ::setsockopt(handle_, SOL_SOCKET, SO_SNDBUF,
				reinterpret_cast<const char*>(&n), sizeof(n));
			if (ret == SOCKET_ERROR) {
				throw WinSockException("failed to set send buffer size socket", ::WSAGetLastError());
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
				throw WinSockException("failed to WSAStartup", ::WSAGetLastError());
			}
		}

		~WinSockInitializer()
		{
			int ret = ::WSACleanup();
			if ( ret != 0 ) {
				// throw WinSockException("failed to WSACleanup", ::WSAGetLastError());
			}
		}
	};
}

#endif
