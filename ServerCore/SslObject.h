#pragma once

#include "Types.h"
#include <openssl/ssl.h>

class SslObject
{
public:
	SslObject(SSL_CTX* ctx, int fd);
	~SslObject();

	SSL*	GetSsl() { return _ssl; }
private:
	int		_fd;
	SSL*	_ssl;
};