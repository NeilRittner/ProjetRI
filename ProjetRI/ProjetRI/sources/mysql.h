#pragma once

#ifdef _WIN32
	#pragma comment(lib, "lib/libmysql.lib")
	#include "socket.h"
	#include "include/mysql.h"
#else
	#include <mysql/mysql.h>
#endif
