#pragma once
#define LISTEN_BACKLOG 10

#include "EpollCore.h"
#include "NetAddress.h"
#include "Types.h"

#include <cstring> // memset
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <unistd.h>

class Listener : public EpollObject
{
public:
	Listener(ServiceRef service, int port);
	virtual ~Listener();

	void	Accept();

	public: //IocpObject Interface
	virtual int			GetFd() { return _listenSocket; }
	virtual void		SetEpollEvent(EpollEvent* epollEvent) { _epollEvent = epollEvent; }
	virtual EpollEvent*	GetEpollEvent() { return _epollEvent; }
	virtual void		Dispatch(uint32_t events);

private:
	int				_listenSocket = -1;
	NetAddress		_addr;
	ServiceRef		_service = nullptr;
	EpollEvent*		_epollEvent = nullptr;
};