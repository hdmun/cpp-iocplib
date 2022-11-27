#ifndef __WIN_SOCK_H__
#define __WIN_SOCK_H__

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

namespace iocplib {

	enum {
		kSocketBufferSize = 1460,
	};

	class WinSock
	{
	public:
		WinSock();
		virtual ~WinSock();

		WinSock(const WinSock&) = delete;
		WinSock operator=(const WinSock&) = delete;

		void Create( int af, int type, int protocol );
		void Bind( const SOCKADDR* pAddr, int len );
		void Listen( int backlog = SOMAXCONN );
		bool Connect( const SOCKADDR* pAddr );
		void Close();

		int Send(void* data, int len, int flags = 0);
		int Recv(char* buffer, int len, int flag);

		void SetAccept( SOCKET listen_socket );
		void SetNonBlocking( bool nonblocking );

		void SetRecvBufferSize( uint32_t size );
		void SetSendBufferSize( uint32_t size );

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
