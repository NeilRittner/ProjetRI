#pragma once

#include "socket.h"
#include <string>

class HttpRequest
{
public:
	HttpRequest() {}
	~HttpRequest() {}
	void GetResponse(SOCKET sd);

private:
	void SetHeader(std::string &AnswerBuf,char *httpCode, const char *mime="text/html");
};
