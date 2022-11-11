#include "pch.h"
#include "iocplib/win_sock.h"

static iocplib::WinSockInitializer g_winsock_init;

int main()
{
	std::cout << "Hello Server!\n";
}
