#ifndef __WINSOCKLIB_SOCKET_IO_THREAD_H__
#define __WINSOCKLIB_SOCKET_IO_THREAD_H__

namespace winsocklib {
	template<typename _Session>
	struct SocketIoThreadRequest {
		std::weak_ptr<_Session> socket;
		long network_events{ ~0L };
	};

	template<typename _Session>
	struct SocketIoThreadSocket {
		std::weak_ptr<_Session> socket;
		WSAEVENT wsa_event;
	};

	template<typename _Session>
	class SocketIoThread {
	public:
		SocketIoThread();
		virtual ~SocketIoThread();
		void Close();

		void RequestRegisterSocket(const std::weak_ptr<_Session> socket, long network_events);
		void RequestUnregisterSocket(const std::weak_ptr<_Session> socket);

	private:
		void Run();

		void OnSocketEvent(WSAEVENT wsa_event);

		void OnRequest();
		void OnRegisterSocket(const std::weak_ptr<_Session>& socket, long network_events);
		void OnUnregisterSocket(const std::weak_ptr<_Session>& socket);

		bool running_;
		std::thread thread_;

		std::mutex requests_lock_;
		std::queue<std::unique_ptr<SocketIoThreadRequest<_Session> > > requests_;

		std::mutex sockets_lock_;
		std::vector<WSAEVENT> events_;
		std::map<WSAEVENT, std::weak_ptr<_Session> > socket_map_;
		std::map<SOCKET, WSAEVENT > event_map_;
	};
}
#include "src/socket_io_thread.cpp"
#endif
