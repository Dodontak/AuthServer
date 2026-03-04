#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <stdlib.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <arpa/inet.h>
#include <iostream>
#include <fstream>

using namespace std;

void	handle_error(const char* errstr)
{
	cout << errstr << endl;
	exit(1);
}

SSL_CTX* create_context()
{
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx)
		handle_error("SSL_CTX_new error");
    return ctx;
}

int	main()
{
	SSL_CTX* ctx = create_context();

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(4242);
	sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(server_socket, (const sockaddr*)&sock_addr, sizeof(sock_addr)))
		handle_error("connect error");

	SSL*	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, server_socket);

	if (SSL_connect(ssl) <= 0)
	{
	        std::cout << "Handshake fail\n";
	        ERR_print_errors_fp(stderr);
        	return -1;
	}

	std::ofstream readFile("readFile.txt");
	std::ofstream writeFile("writeFile.txt");
	
	while (1)
	{
		string str;
		cin >>	str;
		readFile << str << endl;
		readFile.flush();
		SSL_write(ssl, str.c_str(), str.length());
		char	buffer[1001];
		int rd = SSL_read(ssl, buffer, 1000);
		buffer[rd] = 0;
		writeFile << buffer << endl;
		writeFile.flush();
		cout << buffer << endl;
	}
	close(server_socket);
}
