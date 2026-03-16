#pragma once

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "NetAddress.h"

class SocketUtils
{
public:
    static int	CreateSocket();
    static void	CloseSocket(int socket);
    static bool	MakeSocketNonblock(int sock);
    static bool Bind(int socket, NetAddress addr);
    static bool SetReuseAddress(int socket, bool flag);
};

class Utils
{
public:
    static void         ErrorExit(const char* err_str);
    static std::string	GetRandomStr(int len);
    static std::string  CreateAccessToken(const std::string& user_id, const std::string& nickname);
    static bool         VerifyAccessToken(const std::string& token, std::string& out_user_id);
};