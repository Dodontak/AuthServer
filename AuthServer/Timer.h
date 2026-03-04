#pragma once

#include "EpollObject.h"
#include "Types.h"
#include "Service.h"
#include <sys/timerfd.h>
#include <functional>

class Timer : public EpollObject
{
public:
	Timer(std::function<void()> callback, int sec, ServiceRef service);
	~Timer();

	void	ReleaseTimer();
public:
	virtual int			GetFd() { return _tfd; }
	virtual void		SetEpollEvent(EpollEvent* timerEvent) { _timerEvent = timerEvent; }
	virtual EpollEvent*	GetEpollEvent() {return _timerEvent; }
	virtual void		Dispatch(uint32_t events);
private:
	int	_tfd;
	std::function<void()>	_callback;
	itimerspec				_ts{};
	EpollEvent*				_timerEvent = nullptr;
	std::weak_ptr<Service>	_service;
};

