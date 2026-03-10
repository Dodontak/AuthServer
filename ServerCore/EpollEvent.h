#pragma once

#include "Types.h"

enum class EventType : int
{
	Connect,
	Disconnect,
	Accept,
	Read,
	Write,
	HandShakingRead,
	HandShakingWrite,
	Timer
};

class EpollEvent
{
public:
	EpollEvent(EpollObjectRef owner, EventType type);
	~EpollEvent();
	
public:
	EpollObjectRef	GetOwner();
	void			SetEventType(EventType type) { _type = type; }
	EventType		GetEventType() { return _type; }
private:
	void	SetEventType(struct epoll_event &ev, EventType type);

	EventType				_type;
	std::weak_ptr<EpollObject>	_owner;
};
