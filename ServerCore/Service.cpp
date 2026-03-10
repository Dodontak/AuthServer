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

AuthService::AuthService(const char* ip, int port, const char* certFile, const char* keyFile, SessionFactory factory) :
	Service(ip, port, factory)
{
	_ctx = std::make_shared<SslCtx>(true);
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
	Service(ip, port, factory), _clientCount(clientCount)
{
	_ctx = std::make_shared<SslCtx>(false);
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
		SocketUtil::MakeSocketNonblock(socket);

		session->Connect();
		_epollCore->Register(epollEvent);
		InsertSession(session);
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

#include <random>

SessionRef	ClientService::GetRandomSession()
{
	std::random_device rd;
	std::mt19937	gen(rd());
	std::lock_guard<std::mutex>	lock(_m);
	std::set<EpollObjectRef>::iterator it = _sessions.begin();
	int	session_count = _sessions.size();
	if (session_count == 0)
		return nullptr;	
	std::uniform_int_distribution<int> dis(0, session_count - 1);
	int idx = dis(gen);
	for (int i = 0; i < idx; i++)
		++it;
	return static_pointer_cast<Session>(*it);
}
