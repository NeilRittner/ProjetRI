#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "http.h"
#include "params.h"

#define TAILLE 50

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

extern ParamIndex params;

void HttpRequest::GetResponse(SOCKET sd)
{
	char buffer[TAILLE + 1];
	int nb;

	// à vous de jouer ...

	int etat = 0;
	char search_caract[4];
	search_caract[0] = '\r';
	search_caract[1] = '\n';
	search_caract[2] = '\r';
	search_caract[3] = '\n';

	while (etat != 4) {
		nb = recv(sd, buffer, TAILLE, 0);
		for (size_t i = 0; i <= TAILLE; i++) {
			if (buffer[i] == search_caract[etat]) {
				std::cout << etat << std::endl;
				etat = etat + 1;
				if (etat == 4) {
					break;
				}
			}
			else {
				etat = 0;
			}
		}
	}

	nb = send(sd, buffer, TAILLE, 0);
	std::cout << buffer << std::endl;
}

void HttpRequest::SetHeader(std::string &AnswerBuf, char *httpCode, const char *mime)
{
	std::stringstream header;

	// GetTime
	auto t = std::time(nullptr);

	// Set HTTP header
	header << "HTTP/1.0 " << httpCode << "\r\n";
	header << "Date: " << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	header << "Server: WebSearchServer/1.0" << "\r\n";
	header << "Content-Type: " << mime << "\r\n";
	header << "\r\n";

	AnswerBuf = header.str();
}
