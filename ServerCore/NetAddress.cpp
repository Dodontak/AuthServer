#include "NetAddress.h"
#include <cstring>
#include <arpa/inet.h>

using namespace std;

NetAddress::NetAddress(sockaddr_in sockAddr) : _sockAddr(sockAddr) {}

NetAddress::NetAddress(string ip, uint16 port)
{
	memset(&_sockAddr, 0, sizeof(sockaddr_in));

	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_port = htons(port);
	if (ip == "localhost") {
		inet_pton(AF_INET, "127.0.0.1", &_sockAddr.sin_addr);
    } else if (ip.empty()) {
        _sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
		inet_pton(AF_INET, ip.c_str(), &_sockAddr.sin_addr);
    }
}


string	NetAddress::GetIpString()
{
	char	buffer[INET_ADDRSTRLEN]; // IPv4용 버퍼
	const char* result = inet_ntop(
		AF_INET,
		&_sockAddr.sin_addr,
		buffer,
		INET_ADDRSTRLEN
	);
	if (result == nullptr)
		return "";
	return string(buffer);
}
