#include "Utils.h"
#include <iomanip>
#include <vector>
#include <openssl/rand.h>

void handle_error(const char* err_str, int rtn)
{
	cerr << err_str << endl;
	exit(rtn);
}

void SocketUtil::MakeSocketNonblock(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
		handle_error("fcntl F_GETFL error", 1);
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		handle_error("fcntl NONBLOCK error", 1);
}

int	SocketUtil::CreateSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

void	SocketUtil::CloseSocket(int socket)
{
	close(socket);
}

std::string	GetTempId(int len)
{
	std::vector<unsigned char>	buffer(len);
	if (RAND_bytes(buffer.data(), buffer.size()) != 1)
	{
		cout << "error" << endl;
	}
	std::stringstream ss;
    for (unsigned char byte : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
	return ss.str();
}