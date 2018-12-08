#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <csignal>

#ifdef _WIN32
	#include <Windows.h>
#endif

#include "socket.h"
#include "params.h"
#include "index.h"
#include "http.h"
#include "ThreadPool.h"

ParamIndex params;

void ReadConfFile(ParamIndex &params)
{
	int c;
	std::string tmp;
	unsigned int id;

	std::ifstream config("config.txt");
	if (config.is_open())
	{
		id=0;
		while ((c=config.get())!=EOF)
		{
			if (c=='\n')
			{
				if (tmp.size())
				{
					switch(id)
					{
					case 0:params.ServerName=tmp;break;
					case 1:params.Login=tmp;break;
					case 2:params.Password=tmp;break;
					case 3:params.SchemeName=tmp;break;
					case 4:
						if ((tmp[tmp.size()-1]!='/') && (tmp[tmp.size()-1]!='\\')) tmp+='\\';
						params.BaseFiles=tmp;
						break;
					}
					tmp="";
				}
				id++;
			}
			else tmp+=c;
		}
	}
}

void AnswerServer(SOCKET sd)
{
	HttpRequest aRequest;

	aRequest.GetResponse(sd);

	closesocket(sd);
}

namespace
{
volatile SOCKET sd;

void StopServer(int sig)
{
	closesocket(sd);
}
}

void StartServer(unsigned int port)
{
	SOCKET new_conn;	// file descriptor of the socket
	struct sockaddr_in addr;	// address (local port)
	struct sockaddr_in caddr;	// address of the client
	socklen_t caddrlen;	// length of the address

					// set the address to a local port (variable `port')
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(port);

	ThreadPool pool(4);

	// create the socket
	if ((sd=socket(PF_INET, SOCK_STREAM, 0))!=INVALID_SOCKET)
	{
		// bind the socket to the local port
		if (!bind(sd, (struct sockaddr*)&addr, sizeof(addr)))
		{
			// listen to the socket
			if (!listen(sd, SOMAXCONN))
			{
				int optVal;
				int optLen=sizeof(int);

				std::cout << "-> Server listening" << std::endl;
				// accept connections
				do
				{
					caddrlen=sizeof(caddr);
					if ((new_conn=accept(sd, (struct sockaddr*)&caddr, &caddrlen))!=INVALID_SOCKET)
						pool.Execute(AnswerServer, new_conn);
				} while (getsockopt(sd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&optVal, &optLen)!=SOCKET_ERROR);
				std::cout << "-> Stop server listening" << std::endl;
			}
		}

		// closes the server socket
		closesocket(sd);
	}
}

int main(int argc,char *argv[])
{
#ifdef _WIN32
	// Console charset
	SetConsoleOutputCP(GetACP());

	// Init socket under win32
	WSADATA wsaData;
	WORD wVersionRequested=MAKEWORD(1,1);
	if (WSAStartup(wVersionRequested,&wsaData)) return(-1);
	if ((LOBYTE(wsaData.wVersion)!=1) || (HIBYTE(wsaData.wVersion)!=1))
	{
		WSACleanup();
		return(-1);
	}
	// End init socket
#endif

	std::signal(SIGINT, StopServer);
	std::signal(SIGTERM, StopServer);
#ifdef SIGBREAK
	std::signal(SIGBREAK, StopServer);
#endif

	ReadConfFile(params);
	std::cout << "Data from config file" << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	std::cout << "ServerName: " << params.ServerName.c_str() << std::endl;
	std::cout << "Login: " << params.Login.c_str() << std::endl;
	std::cout << "Password: " << params.Password.c_str() << std::endl;
	std::cout << "SchemeName: " << params.SchemeName.c_str() << std::endl;
	std::cout << "BaseFiles: " << params.BaseFiles.c_str() << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	std::cout << std::endl;

	if ((argc==2) && (!strcmp(argv[1],"i")))
		IndexData();
	StartServer(8090);

#ifdef _WIN32
	WSACleanup();
#endif

	return(0);
}
