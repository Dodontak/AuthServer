#include "EpollCore.h"
#include "Listener.h"


EpollCore::EpollCore()
{
	_epfd = epoll_create1(0);
	if (_epfd == -1)
		handle_error("epoll_create1 error", 1);
}

EpollCore::~EpollCore()
{
	close(_epfd);
}

void	EpollCore::StartEpollWait()
{
	while (true)
	{
		int	nfds = epoll_wait(_epfd, _epEvents.data(), MAX_WATCH_EVENT, -1);
		for (int n = 0; n < nfds; n++)
		{
			struct epoll_event	ev = _epEvents[n];
			EpollEvent* 		epollEvent = (EpollEvent*)ev.data.ptr;
			EpollObjectRef		epollObject = epollEvent->GetOwner();
			if (epollObject)
				epollObject->Dispatch(ev.events);
		}
	}
}

//Epoll에 감시 등록하는 함수
void	EpollCore::Register(EpollEvent* epollEvent)
{
	EventType			type = epollEvent->GetEventType();
	EpollObjectRef		epollObject = epollEvent->GetOwner();
	if (epollObject == nullptr)
		return;
	struct epoll_event	ev = CreateEv(epollObject, type);
	epoll_ctl(_epfd, EPOLL_CTL_ADD, epollObject->GetFd(), &ev);
}

void	EpollCore::ModEvent(EpollEvent* epollEvent)
{
	EventType			type = epollEvent->GetEventType();
	EpollObjectRef		epollObject = epollEvent->GetOwner();
	if (epollObject == nullptr)
		return;
	struct epoll_event	ev = CreateEv(epollObject, type);
	epoll_ctl(_epfd, EPOLL_CTL_MOD, epollObject->GetFd(), &ev);
}

void	EpollCore::DelEvent(EpollEvent* epollEvent)
{
	EventType			type = epollEvent->GetEventType();
	EpollObjectRef		epollObject = epollEvent->GetOwner();
	if (epollObject == nullptr)
		return;
	epoll_ctl(_epfd, EPOLL_CTL_DEL, epollObject->GetFd(), nullptr);
}

struct epoll_event	EpollCore::CreateEv(EpollObjectRef epollObject, EventType type)
{
	struct epoll_event	ev;
	memset(&ev, 0, sizeof(struct epoll_event));

	switch (type) {
		case EventType::Connect :
			ev.events = EPOLLOUT;
			break;
		case EventType::Disconnect :
			ev.events = EPOLLIN;
			break;
		case EventType::Accept :
			ev.events = EPOLLIN;
			break;
		case EventType::Read :
			ev.events = EPOLLIN;
			break;
		case EventType::Write :
			ev.events = EPOLLIN | EPOLLOUT;
			break;
		case EventType::HandShakingRead :
			ev.events = EPOLLIN;
			break;
		case EventType::HandShakingWrite :
			ev.events = EPOLLOUT;
			break;
		case EventType::Timer :
			ev.events = EPOLLIN;
			break;
	}
	ev.data.fd = epollObject->GetFd();
	ev.data.ptr = (void*)epollObject->GetEpollEvent();
	return ev;
}
