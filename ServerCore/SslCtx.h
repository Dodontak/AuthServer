#pragma once

#include <openssl/ssl.h>

class SslCtx
{
public:
	SslCtx(bool serverMethod);
	~SslCtx();

	void		SetCrt(const char* crt);
	void		SetKey(const char* key);
	SSL_CTX*	GetCtx() { return _ctx;}
private:
	SSL_CTX*	_ctx;
};