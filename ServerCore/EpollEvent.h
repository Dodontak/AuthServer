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
	EpollObjectRef	GetOwner() { return _owner; }
	void			SetEventType(EventType type) { _type = type; }
	EventType		GetEventType() { return _type; }
	void			ClearOwner() { _owner = nullptr; }
private:
	void	SetEventType(struct epoll_event &ev, EventType type);

	EventType		_type;
	EpollObjectRef	_owner;
};
