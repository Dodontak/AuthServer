#pragma once

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>

class SocketUtil
{
public:
	static void	MakeSocketNonblock(int sock);
	static int	CreateSocket();
	static void	CloseSocket(int socket);
};

void 		handle_error(const char* err_str, int rtn);
std::string	GetTempId(int len);
std::string CreateAccessToken(const std::string& user_id, const std::string& nickname);
bool VerifyAccessToken(const std::string& token, std::string& out_user_id);