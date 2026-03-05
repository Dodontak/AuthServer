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

using SessionFactory = function<SessionRef(int, struct sockaddr_in, ServiceRef)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(std::string ip, int port, const char* certFile, const char* keyFile, SessionFactory factory);
	virtual ~Service() {}

	SessionRef	MakeSession(int clinetSocket, struct sockaddr_in addr, ServiceRef service);

	void	InsertSession(EpollObjectRef session);
	void	EraseSession(EpollObjectRef session);

	EpollCoreRef	GetEpollCore() { return _epollCore; }
	NetAddress		GetNetAddress() { return _addr; }
	SSL_CTX*		GetCtx();

	int				Start();
protected:
	std::mutex	m;
	EpollCoreRef		_epollCore;
	SslCtxRef			_ctx;
	int 				_port;
	NetAddress			_addr;
	set<EpollObjectRef>	_sessions;
	SessionFactory		_sessionFactory;
};