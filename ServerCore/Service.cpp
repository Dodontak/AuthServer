#include "Service.h"
#include "NetAddress.h"
#include "SslCtx.h"
#include "Listener.h"
#include "Session.h"
#include <signal.h>
#include <memory>

using namespace std;

/*====================
       Service
====================*/

Service::Service(NetAddress addr, SessionFactory factory)
	: _addr(addr), _sessionFactory(factory)
{
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
	if (session)
		_sessions.erase(session);
}

/*====================
     AuthService
====================*/

AuthService::AuthService(NetAddress addr, const char* certFile, const char* keyFile, SessionFactory factory) :
	Service(addr, factory)
{
	_ctx = make_shared<SslCtx>(true);
	_ctx->SetCrt(certFile);
	_ctx->SetKey(keyFile);
}

AuthService::~AuthService() {}

int	AuthService::Start()
{
	signal(SIGPIPE, SIG_IGN);
	ListenerRef		listener = make_shared<Listener>(shared_from_this());
	EpollEvent*		listenEvent = new EpollEvent(listener, EventType::Accept);

	_epollCore = make_shared<EpollCore>();
	_epollCore->Register(listenEvent);
	_epollCore->StartEpollWait();
	return 1;
}

/*====================
     ClientService
====================*/


ClientService::ClientService(NetAddress addr, SessionFactory factory, int clientCount) :
	Service(addr, factory), _clientCount(clientCount)
{
	_ctx = make_shared<SslCtx>(false);
}

ClientService::~ClientService() {}

int	ClientService::Start()
{
	signal(SIGPIPE, SIG_IGN);
	
	_epollCore = make_shared<EpollCore>();
	for (int i = 0; i < _clientCount; ++i)
	{
		int	socket = SocketUtil::CreateSocket();
		int	addrlen = sizeof(sockaddr_in);
		SessionRef		session = _sessionFactory(socket, _addr.GetAddr(), shared_from_this());
		EpollEvent*		epollEvent = new EpollEvent(session, EventType::Connect);
		if (false == SocketUtil::MakeSocketNonblock(socket))
            handle_error("ClientService Start MakeSocketNonblock error", 1);

		session->Connect();
		_epollCore->Register(epollEvent);
		InsertSession(session);
	}
	_epollCore->StartEpollWait();
	return 1;
}

void	ClientService::broadcastfortest(WriteBufferRef writebuffer)
{
	lock_guard<mutex>	lock(_m);
	for (auto session : _sessions)
	{
		static_pointer_cast<Session>(session)->Send(writebuffer);
	}
}

#include <random>

SessionRef	ClientService::GetRandomSession()
{
	random_device rd;
	mt19937	gen(rd());
	lock_guard<mutex>	lock(_m);
	set<EpollObjectRef>::iterator it = _sessions.begin();
	int	session_count = _sessions.size();
	if (session_count == 0)
		return nullptr;	
	uniform_int_distribution<int> dis(0, session_count - 1);
	int idx = dis(gen);
	for (int i = 0; i < idx; i++)
		++it;
	return static_pointer_cast<Session>(*it);
}
