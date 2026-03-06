#pragma once

#include "Types.h"
#include <openssl/ssl.h>

class SslObject
{
public:
	SslObject(SSL_CTX* ctx, int fd);
	~SslObject();

	bool	Init();
	int		SslAccept();
	int		SslConnect();
	SSL*	GetSsl() { return _ssl; }
private:
	SSL_CTX*	_ctx;
	int			_fd;
	SSL*		_ssl;
};