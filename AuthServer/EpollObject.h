#pragma once
#include <memory>

class EpollEvent;

class EpollObject
{
public:
	virtual int			GetFd() = 0;
	virtual void		SetEpollEvent(EpollEvent* epollEvent) = 0;
	virtual EpollEvent*	GetEpollEvent() = 0;
	virtual void		Dispatch(uint32_t events) = 0;
};