#include "EpollEvent.h"
#include "EpollCore.h"

EpollEvent::EpollEvent(EpollObjectRef owner, EventType type) : _owner(owner), _type(type)
{
	cout << "EpollEvent constructed." << endl;
	owner->SetEpollEvent(this);
}

EpollEvent::~EpollEvent()
{
	cout << "EpollEvent distructed." << endl;
}

EpollObjectRef	EpollEvent::GetOwner()
{
	return _owner.lock();
}
