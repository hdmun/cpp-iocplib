#ifndef __IO_COMPLETION_PORT_H__
#define __IO_COMPLETION_PORT_H__
#include "win_sock.h"

namespace iocplib {
	class IoCompletionPortWorker
	{
	public:
		IoCompletionPortWorker(IoCompletionPortWorker&) = delete;
		IoCompletionPortWorker operator= (IoCompletionPortWorker&) = delete;

		IoCompletionPortWorker(HANDLE iocp_handle);
		virtual ~IoCompletionPortWorker() {}

		void Run();
		void Join();

	private:
		HANDLE iocp_handle_{ nullptr };
		std::thread thread_;
	};

	class IoCompletionPort
	{
	public:
		IoCompletionPort(IoCompletionPort&) = delete;
		IoCompletionPort operator= (IoCompletionPort&) = delete;

		IoCompletionPort() {}
		virtual ~IoCompletionPort() {}

		HANDLE handle() const {
			return handle_;
		}

		bool Create(uint32_t io_thread = 0U);
		void Close();

		void Attach(HANDLE socket);
		void Detach();

	private:
		HANDLE handle_{ nullptr };

		std::mutex worker_lock_;
		std::vector< std::unique_ptr<IoCompletionPortWorker> > workers_;

		static std::atomic<uint32_t> attached_sockets_;
	};
}
#endif