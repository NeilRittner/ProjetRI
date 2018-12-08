#pragma once

#ifdef _WIN32
	#include <Windows.h>
	#pragma comment(lib, "ws2_32.lib")
	using socklen_t=int;
#else
	using SOCKET=int;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/ip.h>
	#include <unistd.h>
	#define closesocket(s) close(s)
#endif
