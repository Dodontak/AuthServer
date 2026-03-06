#include "Service.h"
#include "SslCtx.h"
#include "Listener.h"
#include "Session.h"
#include <signal.h>
#include <memory>

/*====================
       Service
====================*/

Service::Service(const char* ip, int port, SessionFactory factory)
	: _addr(ip, port), _sessionFactory(factory)
{
	_ctx = std::make_shared<SslCtx>();
}

SessionRef	Service::MakeSession(int clinetSocket, struct sockaddr_in addr, ServiceRef service)
{
	return _sessionFactory(clinetSocket, addr, service);
}


SSL_CTX*	Service::GetCtx()
{
	return _ctx->GetCtx();
}

void	Service::InsertSession(EpollObjectRef session)
{
	_sessions.insert(session);
}

void	Service::EraseSession(EpollObjectRef session)
{
	_sessions.erase(session);
}

/*====================
     AuthService
====================*/

AuthService::AuthService(const char* ip, int port, const char* certFile, const char* keyFile, SessionFactory factory) :
	Service(ip, port, factory)
{

	_ctx->SetCrt(certFile);
	_ctx->SetKey(keyFile);
}

AuthService::~AuthService()
{

}

int	AuthService::Start()
{
	signal(SIGPIPE, SIG_IGN);
	ListenerRef		listener = make_shared<Listener>(shared_from_this(), _addr.GetPort());
	EpollEvent*		listenEvent = new EpollEvent(listener, EventType::Accept);

	_epollCore = make_shared<EpollCore>();
	_epollCore->Register(listenEvent);
	_epollCore->StartEpollWait();
	return 1;
}

/*====================
     ClientService
====================*/


ClientService::ClientService(const char* ip, int port, SessionFactory factory, int clientCount) :
	Service(ip, port, factory), _clientCount(clientCount) {}

ClientService::~ClientService() {}

int	ClientService::Start()
{
	signal(SIGPIPE, SIG_IGN);
	for (int i = 0; i < _clientCount; i++)
	{
	}
	_epollCore->StartEpollWait();
	return 1;
}

void	ClientService::broadcastfortest(WriteBufferRef writebuffer)
{
	std::lock_guard<std::mutex>	lock(_m);
	for (auto session : _sessions)
	{
		static_pointer_cast<Session>(session)->Send(writebuffer);
	}
}
