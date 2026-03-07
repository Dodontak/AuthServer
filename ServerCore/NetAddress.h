#pragma once

#include "Types.h"
#include <arpa/inet.h>
#include <string>

struct sockaddr_in;

class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(sockaddr_in sockAddr);
	NetAddress(std::string ip, uint16 port);

	sockaddr_in		&GetAddr() { return _sockAddr; }
	int				GetPort() { return ntohs(_sockAddr.sin_port); }
	std::string		GetIpString();
private:
	sockaddr_in		_sockAddr = {};
};