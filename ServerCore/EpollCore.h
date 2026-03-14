#pragma once

#include "Utils.h"
#include "Types.h"
#include "EpollEvent.h"
#include "EpollObject.h"
#include <sys/epoll.h>
#include <array>

class EpollCore : public std::enable_shared_from_this<EpollCore>
{
	enum { MAX_WATCH_EVENT = 10 };
public:
	EpollCore();
	~EpollCore();

	int		GetFd() { return _epfd; }
public:
	void	StartEpollWait();
	void	Register(EpollEvent* epollEvent);
	void	ModEvent(EpollEvent* epollEvent);
	void	DelEvent(EpollEvent* epollEvent);

private:
	struct epoll_event	CreateEv(EpollObjectRef epollObject, EventType type);

private:
	std::array<struct epoll_event, MAX_WATCH_EVENT>	_epEvents;
	int											_epfd;
};