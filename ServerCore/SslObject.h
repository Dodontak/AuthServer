#pragma once

#include "Types.h"
#include <openssl/ssl.h>

class SslObject
{
public:
	SslObject(SSL_CTX* ctx, int fd);
	~SslObject();

	bool	Init();
	int		Accept();
	int		Connect();
	int		Read(BYTE* buffer, int readSize, size_t* readLen);
	int		HasPendingData();
	int		Write(BYTE* buffer, int dataLen, size_t* writeLen);
	SSL*	GetSsl() { return _ssl; }
private:
	SSL_CTX*	_ctx;
	int			_fd;
	SSL*		_ssl;
};