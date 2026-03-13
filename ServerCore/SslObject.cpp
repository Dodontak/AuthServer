#include "SslObject.h"
#include "EpollObject.h"
#include "Utils.h"

#include <openssl/err.h>

SslObject::SslObject(SSL_CTX* ctx, int fd) : _ctx(ctx), _fd(fd), _ssl(nullptr) {}

// 리턴 0 성공. 1 진행중 read 대기 필요. 2 진행중 write 대기 필요. -1 에러 세션종료 필요.
int	SslObject::Accept()
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
int	SslObject::Connect()
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
//0 성공, 1 복호화 데이터 부족, 2 상대 연결 종료, 3 SSL 에러
int		SslObject::Read(BYTE* buffer, int readSize, size_t* readLen)
{
	int	ret = SSL_read_ex(_ssl, buffer, readSize, readLen);
	if (ret == 0)//실패
	{
		int err = SSL_get_error(_ssl, ret);
		if (err == SSL_ERROR_WANT_READ){//복호화 하기에 데이터 부족함.
			return 1;
		} else if (err == SSL_ERROR_ZERO_RETURN) {
			// 상대가 close 함. SSL_shutdown ok
			return 2;
		} else {//SSL에 심각한 에러 발생. SSL_shutdown 금지.
			return 3;
		}
	}
	return 0;
}

int	SslObject::HasPendingData()
{
	return SSL_has_pending(_ssl);
}

//0 성공, 1 일부만 성공, 2 커널 버퍼 공간 부족, 3 상대 연결 종료, 4 SSL 에러
int	SslObject::Write(BYTE* buffer, int dataLen, size_t* writeLen)
{
	int	ret = SSL_write_ex(_ssl, buffer, dataLen, writeLen);
	if (ret == 0)
	{// 실패
		int err = SSL_get_error(_ssl, ret);
		if (err == SSL_ERROR_WANT_WRITE){//커널 버퍼 공간 부족
			return 2;
		} else if (err == SSL_ERROR_ZERO_RETURN) {
			// 상대가 연결 종료함
			return 3;
		} else {//SSL에 심각한 에러 발생. SSL_shutdown 금지.
			return 4;
		}
	}
	else if (*writeLen != dataLen)
	{//일부만 write 성공 시
		return 1;
	}
	return 0;
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