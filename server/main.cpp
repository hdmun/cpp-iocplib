﻿#include "pch.h"
#include <csignal>

#include "iocplib/win_sock.h"

static iocplib::WinSockInitializer g_winsock_init;
static bool g_running = true;

static void SignalHandler(int signo)
{
	switch (signo)
	{
	case SIGTERM:
		g_running = false;
		break;
	case SIGINT:
		g_running = false;
		break;
	default:
		break;
	}
}

int main()
{
	// https://tttsss77.tistory.com/212
	if (signal(SIGTERM, SignalHandler) == SIG_ERR) {
		return -1;
	}
	if (signal(SIGINT, SignalHandler) == SIG_ERR) {
		return -1;
	}

	while (g_running) {
		::Sleep(1000);
	}

	return 0;
}
