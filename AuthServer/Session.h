#pragma once

#include "EpollObject.h"
#include "SslObject.h"
#include "NetAddress.h"
#include "ReadBuffer.h"
#include "WriteBuffer.h"
#include "EpollEvent.h"
#include <deque>
#include <mutex>
#include <memory>

class Service;

class Session : public EpollObject, public std::enable_shared_from_this<Session>
{
	enum { BUFFER_SIZE = 0x2000, READ_SIZE = 0x0400 }; //8kb, 1kb
public:
	Session(int clinetSocket, struct sockaddr_in addr, ServiceRef service);
	virtual ~Session();

	NetAddress	GetNetAddress() { return _address; }
	void		SetSslObject(SslObjectRef sslObject);

public: //IocpObject Interface
	virtual int			GetFd() { return _clientSocket;}
	virtual void		SetEpollEvent(EpollEvent* epollEvent) { _epollEvent = epollEvent; }
	virtual EpollEvent*	GetEpollEvent() { return _epollEvent; }
	virtual void		Dispatch(uint32_t events);

protected:// 컨텐츠 코드에서 오버라이딩
	virtual int		OnRead(BYTE* buffer, int len) { return len; }
	virtual void	OnWrite(int len) {}

private:
	void	Disconnect();
	void	Send(WriteBufferRef writeBuffer);

	void	ProcessRead();
	void	ProcessWrite();
	void	ProcessConnect();
	void	ProcessDisconnect(bool isCanSslShutdown);

	void	ModEvent(EventType type);
	ReadBuffer					_readBuffer;
private:
	std::mutex					m;

	int							_clientSocket;
	NetAddress					_address;
	SslObjectRef				_sslObject = nullptr;
	std::weak_ptr<Service>		_service;
	EpollEvent*					_epollEvent = nullptr;
	
	int							timerfd = -1;
	EpollEvent*					_epollTimerEvent = nullptr;
	std::deque<WriteBufferRef>	_writeBuffers;
};


struct PacketHeader
{
	uint16	size;
	uint16	id;
};

class PacketSession : public Session
{
public:
	PacketSession(int clinetSocket, struct sockaddr_in addr, ServiceRef service);
	virtual ~PacketSession();
protected:
	virtual int		OnRead(BYTE* buffer, int len) final;
	virtual void	OnReadPacket(BYTE* buffer, int len) = 0;
};
