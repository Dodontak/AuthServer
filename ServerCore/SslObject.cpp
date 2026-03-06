#include "SslObject.h"
#include "EpollObject.h"
#include "Utils.h"

SslObject::SslObject(SSL_CTX* ctx, int fd) : _ctx(ctx), _fd(fd), _ssl(nullptr) {}

// 리턴 0 성공. 1 진행중 read 대기 필요. 2 진행중 write 대기 필요. -1 에러 세션종료 필요.
int	SslObject::SslAccept()
{
	if (Init() == false)
		return -1;
		
	int ret = SSL_accept(_ssl);
	if (ret == 1)
		return 0;

	int	err = SSL_get_error(_ssl, ret);
	if (err == SSL_ERROR_WANT_READ)
		return 1;
	else if (err == SSL_ERROR_WANT_WRITE)
		return 2;
	return -1;
}

// 리턴 0 성공. 1 진행중 read 대기 필요. 2 진행중 write 대기 필요. -1 에러 세션종료 필요.
int	SslObject::SslConnect()
{
	if (Init() == false)
		return -1;

	int ret = SSL_connect(_ssl);
	if (ret == 1)
		return 0;
		
	int err = SSL_get_error(_ssl, ret);
	if (err == SSL_ERROR_WANT_READ)
		return 1;
	else if (err == SSL_ERROR_WANT_WRITE)
		return 2;
	return -1;
}

bool	SslObject::Init()
{
	if (_ssl == nullptr)
	{ // ssl 아직 없으면 만들기
		_ssl = SSL_new(_ctx);
		if (_ssl == nullptr)
			return false; //ssl생성 실패
		if (SSL_set_fd(_ssl, _fd) == 0)
		{
			SSL_free(_ssl);
			_ssl = nullptr;
			return false;
		}
	}
	return true;
}

SslObject::~SslObject()
{
	if (_ssl != nullptr)
		SSL_free(_ssl);
}