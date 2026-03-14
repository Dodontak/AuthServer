#pragma once

#include "EpollCore.h"
#include "NetAddress.h"
#include "Types.h"
#include "Session.h"
#include <openssl/ssl.h>
#include <string>
#include <set>
#include <mutex>
#include <functional>

using SessionFactory = std::function<SessionRef(int, struct sockaddr_in, ServiceRef)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(const char* ip, int port, SessionFactory factory);
	virtual ~Service() {}

	SessionRef	MakeSession(int clinetSocket, struct sockaddr_in addr, ServiceRef service);

	void	InsertSession(EpollObjectRef session);
	void	EraseSession(EpollObjectRef session);

	EpollCoreRef	GetEpollCore() { return _epollCore; }
	NetAddress		GetNetAddress() { return _addr; }
	SSL_CTX*		GetCtx();

	virtual int		Start() = 0;
protected:
	std::mutex					_m;
	EpollCoreRef				_epollCore;
	SslCtxRef					_ctx;
	int 						_port;
	NetAddress					_addr;
	std::set<EpollObjectRef>	_sessions;
	SessionFactory				_sessionFactory;
};

class AuthService : public Service
{
public:
	AuthService(const char* ip, int port, const char* certFile, const char* keyFile, SessionFactory factory);
	virtual ~AuthService();

	virtual int	Start();
};

class ClientService : public Service
{
public:
	ClientService(const char* ip, int port, SessionFactory factory, int clientCount);
	
	void	broadcastfortest(WriteBufferRef writebuffer);
	SessionRef	GetRandomSession();

	virtual ~ClientService();

	virtual int	Start();
private:
	int	_clientCount;
};