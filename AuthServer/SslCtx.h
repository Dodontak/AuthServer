#pragma once

#include <openssl/ssl.h>

class SslCtx
{
public:
	SslCtx(const char* crt, const char* key);
	~SslCtx();

	SSL_CTX*	GetCtx() { return _ctx;}
private:
	SSL_CTX*	_ctx;
};