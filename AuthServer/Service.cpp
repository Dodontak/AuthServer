#include "Service.h"
#include "SslCtx.h"
#include "Listener.h"
#include "Session.h"
#include <signal.h>
#include <memory>


Service::Service(std::string ip, int port, const char* certFile, const char* keyFile)
	: _addr(ip, port)
{
	_ctx = make_shared<SslCtx>(certFile, keyFile);
}

SSL_CTX*	Service::GetCtx()
{
	return _ctx->GetCtx();
}

int	Service::Start()
{
	signal(SIGPIPE, SIG_IGN);
	ListenerRef		listener = make_shared<Listener>(shared_from_this(), _addr.GetPort());
	EpollEvent*		listenEvent = new EpollEvent(listener, EventType::Accept);
	
	_epollCore = make_shared<EpollCore>();
	_epollCore->Register(listenEvent);
	_epollCore->StartEpollWait();
	return 1;
}

void	Service::InsertSession(EpollObjectRef session)
{
	_sessions.insert(session);
}

void	Service::EraseSession(EpollObjectRef session)
{
	_sessions.erase(session);
}
