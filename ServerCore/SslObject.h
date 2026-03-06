#pragma once

#include "Types.h"
#include <openssl/ssl.h>

class SslObject
{
public:
	SslObject(SSL_CTX* ctx, int fd);
	~SslObject();

	int	SslHandShake();
	SSL*	GetSsl() { return _ssl; }
private:
	SSL_CTX*	_ctx;
	int			_fd;
	SSL*		_ssl;
};