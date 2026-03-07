#include "SslCtx.h"
#include "Utils.h"

SslCtx::SslCtx(bool serverMethod)
{
	const SSL_METHOD*	method;
	if (serverMethod)
		method = TLS_server_method();
	else
		method = TLS_client_method();
	_ctx = SSL_CTX_new(method);
	if (!_ctx)
		handle_error("SSL_CTX_new error", 1);

}

void	SslCtx::SetCrt(const char* crt)
{
    if (SSL_CTX_use_certificate_file(_ctx, crt, SSL_FILETYPE_PEM) <= 0)
    	handle_error("SSL_CTX_use_certificate_file error", 1);
}

void	SslCtx::SetKey(const char* key)
{
    if (SSL_CTX_use_PrivateKey_file(_ctx, key, SSL_FILETYPE_PEM) <= 0)
    	handle_error("SSL_CTX_use_PrivateKey_file error", 1);
}

SslCtx::~SslCtx()
{
	SSL_CTX_free(_ctx);
}