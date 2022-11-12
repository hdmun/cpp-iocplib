#include "pch.h"
#include "../win_sock.h"


namespace iocplib {

	WinSock::WinSock()
		: handle_( INVALID_SOCKET )
	{
	}

	WinSock::~WinSock()
	{
	}

	void WinSock::Create( int af, int type, int protocol )
	{
		handle_ = ::socket( af, type, protocol );
		if ( handle_ == INVALID_SOCKET ) {
			std::string msg = "failed create socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	void WinSock::Bind( const SOCKADDR* pAddr, int len )
	{
		int bind_ = ::bind( handle_, pAddr, len );
		if ( bind_ == SOCKET_ERROR ) {
			std::string msg = "failed bind socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	void WinSock::Listen( int backlog /*= SOMAXCONN */ )
	{
		int bind_ = ::listen( handle_, backlog );
		if ( bind_ == SOCKET_ERROR ) {
			std::string msg = "failed listen socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	bool WinSock::Connect( const SOCKADDR* pAddr )
	{
		int ret = ::connect( handle_, pAddr, sizeof( *pAddr ) );
		if ( ret == SOCKET_ERROR || ::WSAGetLastError() == WSAEWOULDBLOCK ) {
			return false;
		}
		return true;
	}

	void WinSock::SetAccept( SOCKET listen_socket )
	{
		int ret = ::setsockopt( handle_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			reinterpret_cast<const char*>(&listen_socket), sizeof( listen_socket )
		);
		if ( ret == SOCKET_ERROR ) {
			std::string msg = "failed update accept socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	void WinSock::SetNonBlocking( bool nonblocking )
	{
		u_long b = nonblocking ? 1UL : 0UL;
		int ret = ::ioctlsocket( handle_, FIONBIO, &b );
		if ( ret == SOCKET_ERROR ) {
			std::string msg = "failed set non blocking socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	void WinSock::SetRecvBufferSize( uint32_t size )
	{
		int n = size;
		int ret = ::setsockopt( handle_, SOL_SOCKET, SO_RCVBUF,
			reinterpret_cast<const char*>(&n), sizeof( n ) );
		if ( ret == SOCKET_ERROR ) {
			std::string msg = "failed set receive buffer size socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

	void WinSock::SetSendBufferSize( uint32_t size )
	{
		int n = size;
		int ret = ::setsockopt( handle_, SOL_SOCKET, SO_SNDBUF,
			reinterpret_cast<const char*>(&n), sizeof( n ) );
		if ( ret == SOCKET_ERROR ) {
			std::string msg = "failed set send buffer size socket, " + std::to_string( ::WSAGetLastError() );
			throw std::exception( msg.c_str() );
		}
	}

}