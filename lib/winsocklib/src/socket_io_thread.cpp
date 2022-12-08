#ifndef __WINSOCKLIB_SOCKET_IO_THREAD_CPP__
#define __WINSOCKLIB_SOCKET_IO_THREAD_CPP__

namespace winsocklib {
	template<typename _Session>
	SocketIoThread<_Session>::SocketIoThread()
		: running_(true)
		, thread_(&SocketIoThread<_Session>::Run, this)
	{
		events_.push_back(::WSACreateEvent());
	}

	template<typename _Session>
	SocketIoThread<_Session>::~SocketIoThread()
	{
		Close();
	}

	template<typename _Session>
	void SocketIoThread<_Session>::Close()
	{
		if (running_) {
			running_ = false;
			::WSASetEvent(events_[0]);
			thread_.join();
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::RequestRegisterSocket(const std::weak_ptr<_Session> socket, long network_events)
	{
		auto request = std::make_unique<SocketIoThreadRequest<_Session> >();
		request->socket = socket;
		request->network_events = network_events;

		{
			std::lock_guard<std::mutex> lock(requests_lock_);
			requests_.push(std::move(request));
			::WSASetEvent(events_[0]);
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::RequestUnregisterSocket(const std::weak_ptr<_Session> socket)
	{
		auto request = std::make_unique<SocketIoThreadRequest<_Session> >();
		request->socket = socket;
		request->network_events = ~0UL;

		{
			std::lock_guard<std::mutex> lock(requests_lock_);
			requests_.push(std::move(request));
			::WSASetEvent(events_[0]);
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::Run()
	{
		while (running_) {
			DWORD cEvents = static_cast<DWORD>(events_.size());
			DWORD ret = ::WSAWaitForMultipleEvents(cEvents, events_.data(), FALSE, INFINITE, FALSE);
			if (ret >= events_.size()) {
				continue;  // todo exception
			}
			
			// notify request
			if (ret == 0) {
				::WSAResetEvent(events_[0]);
				OnRequest();
				ret = 1;
			}

			for (DWORD i = ret; i < cEvents; i++) {
				const WSAEVENT& wsa_event = events_[i];
				DWORD ret = ::WSAWaitForMultipleEvents(1, &wsa_event, FALSE, 0, FALSE);
				if (ret == WSA_WAIT_EVENT_0) {
					OnSocketEvent(wsa_event);
				}
			}
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::OnSocketEvent(WSAEVENT wsa_event)
	{
		std::shared_ptr<_Session> socket;
		{
			std::lock_guard<std::mutex> lock(sockets_lock_);
			const auto iter = socket_map_.find(wsa_event);
			socket = iter->second.lock();
		}

		WSANETWORKEVENTS wevents;
		int ret = ::WSAEnumNetworkEvents(socket->GetSocketHandle(), wsa_event, &wevents);
		if (ret == SOCKET_ERROR) {
			return; // exception?
		}

		if (wevents.lNetworkEvents & FD_READ) {
			socket->OnSocketRead(wevents.iErrorCode[FD_READ_BIT]);
		}
		if (wevents.lNetworkEvents & FD_WRITE) {
			socket->OnSocketWrite(wevents.iErrorCode[FD_WRITE_BIT]);
		}
		if (wevents.lNetworkEvents & FD_CLOSE) {
			socket->OnSocketClose(wevents.iErrorCode[FD_CLOSE_BIT]);
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::OnRequest()
	{
		std::unique_ptr<SocketIoThreadRequest<_Session> > request;
		{
			std::lock_guard<std::mutex> lock(requests_lock_);
			if (requests_.empty()) {
				return;
			}

			request = std::move(requests_.front());
			requests_.pop();
		}

		if (request->network_events == ~0L) {
			OnUnregisterSocket(request->socket);
		}
		else {
			OnRegisterSocket(request->socket, request->network_events);
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::OnRegisterSocket(const std::weak_ptr<_Session>& socket, long network_events)
	{
		const std::shared_ptr<_Session> socket_ptr = socket.lock();
		if (socket_ptr == nullptr) {
			return;
		}

		const WSAEVENT socket_event = ::WSACreateEvent();
		if (socket_event == WSA_INVALID_EVENT) {
			// close socket
			return;
		}

		SOCKET socket_handle = socket_ptr->GetSocketHandle();
		{
			std::lock_guard<std::mutex> lock(sockets_lock_);
			events_.push_back(socket_event);
			socket_map_.insert({ socket_event, socket });
			event_map_.insert({ socket_handle, socket_event });
		}

		int error = ::WSAEventSelect(socket_handle, socket_event, network_events);
		if (error == SOCKET_ERROR) {
			// close socket
		}
	}

	template<typename _Session>
	void SocketIoThread<_Session>::OnUnregisterSocket(const std::weak_ptr<_Session>& socket)
	{
		const std::shared_ptr<_Session> socket_ptr = socket.lock();
		if (socket_ptr == nullptr) {
			return;
		}

		const SOCKET handle = socket_ptr->GetSocketHandle();
		{
			std::lock_guard<std::mutex> lock(sockets_lock_);

			const auto event_map_iter = event_map_.find(handle);
			if (event_map_iter != event_map_.end()) {
				const WSAEVENT socket_event = event_map_iter->second;

				for (auto iter = events_.begin(); iter != events_.end(); iter++) {
					if (*iter == socket_event) {
						events_.erase(iter);
						break;
					}
				}

				const auto socket_map_iter = socket_map_.find(socket_event);
				if (socket_map_iter != socket_map_.end()) {
					socket_map_.erase(socket_event);
				}

				event_map_.erase(handle);
			}
		}
	}
}
#endif