#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include "win_sock.h"
#include "overlapped.h"

namespace iocplib {
	class IoCompletionPort;
	struct OverlappedEventInterface;
	struct OverlappedContext;

	class AcceptSocket
		: public OverlappedContext
	{
	public:
		AcceptSocket();
		virtual ~AcceptSocket() {}

		void PostAccept(SOCKET listen_socket);
		LPSOCKADDR GetRemoteAddr();
		void OnAccept(SOCKET listen_socket);

		std::shared_ptr< WinSock > socket() const { return socket_;  }

	private:
		enum {
			kAddressBufferSize = 16 + sizeof(SOCKADDR_IN),
		};

		TCHAR accept_addr_buffer_[kAddressBufferSize * 2];
		std::shared_ptr< WinSock > socket_;
	};

	class Acceptor
		: public OverlappedEventInterface
	{
	public:
		Acceptor();
		virtual ~Acceptor();

		bool Open(uint32_t io_thread, const SOCKADDR_IN* pAddr, int backlog = SOMAXCONN, int accepts = 1);
		void Close();

		virtual void OnComplete(void* data, DWORD dwError, DWORD dwBytesTransferred, ULONG_PTR completionKey);
		void OnAccept(DWORD dwError, std::shared_ptr<WinSock> socket, const SOCKADDR_IN* pAddr);

	private:
		std::unique_ptr< IoCompletionPort > io_completion_port_;
		WinSock listen_socket_;

		std::vector< std::unique_ptr<AcceptSocket> > accept_sockets_;

		std::mutex sockets_lock_;
		std::list< std::shared_ptr< WinSock > > sockets_;  // connections
	};
}
#endif