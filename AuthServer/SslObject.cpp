#include "SslObject.h"
#include "EpollObject.h"
#include "Utils.h"

SslObject::SslObject(SSL_CTX* ctx, int fd) : _fd(fd)
{
	_ssl = SSL_new(ctx);
	SSL_set_fd(_ssl, fd);
	int rtn = SSL_accept(_ssl);
	if (rtn == 1)
		return;
	int	err = SSL_get_error(_ssl, rtn);
	if (err == SSL_ERROR_WANT_ACCEPT) {
		return;
	}
}

SslObject::~SslObject()
{
	SSL_free(_ssl);
}