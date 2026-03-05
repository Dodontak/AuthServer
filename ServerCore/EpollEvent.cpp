#include "EpollEvent.h"
#include "EpollCore.h"

EpollEvent::EpollEvent(EpollObjectRef owner, EventType type) : _owner(owner), _type(type)
{
	cout << "EpollEvent constructed." << endl;
	_owner->SetEpollEvent(this);
}

EpollEvent::~EpollEvent()
{
	cout << "EpollEvent distructed." << endl;
	_owner = nullptr;
}