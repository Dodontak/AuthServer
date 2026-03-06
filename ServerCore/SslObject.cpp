#include "SslObject.h"
#include "EpollObject.h"
#include "Utils.h"

SslObject::SslObject(SSL_CTX* ctx, int fd) : _ctx(ctx), _fd(fd), _ssl(nullptr) {}

// 리턴 0 성공. 1 진행중 재시도 필요 , -1 에러 세션종료 필요
int	SslObject::SslHandShake()
{
	if (_ssl == nullptr)
	{ // ssl 아직 없으면 만들기
		_ssl = SSL_new(_ctx);
		if (_ssl == nullptr)
			return -1; //ssl생성 실패
		if (SSL_set_fd(_ssl, _fd) == 0)
		{
			SSL_free(_ssl);
			_ssl = nullptr;
			return -1;
		}
	}

	int ret = SSL_accept(_ssl);
	if (ret == 1)
		return 0;

	int	err = SSL_get_error(_ssl, ret);
	if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
		return 1;
	return -1;
}

SslObject::~SslObject()
{
	if (_ssl != nullptr)
		SSL_free(_ssl);
}