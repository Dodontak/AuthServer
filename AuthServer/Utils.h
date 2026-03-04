#pragma once

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace std;


class SocketUtil
{
public:
	static void	MakeSocketNonblock(int sock);
	static int	CreateSocket();
	static void	CloseSocket(int socket);
};

void handle_error(const char* err_str, int rtn);
