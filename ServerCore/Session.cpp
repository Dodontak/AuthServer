#include "CoreGlobal.h"
#include "Threadmanager.h"
#include "Session.h"
#include "Service.h"
#include "Timer.h"
#include "JobQueue.h"
#include <openssl/err.h>
#include <unistd.h>
#include <sys/uio.h>

/*====================
        Session
====================*/

Session::Session(int clientSocket, struct sockaddr_in addr, ServiceRef service) :
	_clientSocket(clientSocket), _address(addr), _service(service), _readBuffer(BUFFER_SIZE)
{
	cout << "Session " << clientSocket <<" Constructed." << endl;
}

Session::~Session()
{
	std::cout << "Session " << _clientSocket << " Distructed." << std::endl;
	close(_clientSocket);
}

void	Session::SetSslObject(SslObjectRef sslObject)
{
	_sslObject = sslObject;
}

void	Session::Dispatch(uint32_t events)
{
	const uint32_t	readWrite = EPOLLIN | EPOLLOUT;
	EventType 		type = _epollEvent->GetEventType();

	if ((events & readWrite) == 0)
	{
		ProcessDisconnect(false);
		return;
	}
	if (type == EventType::Disconnect)
	{
		ProcessDisconnect(true);
		return;
	}
	if (events & EPOLLIN)
	{
		ProcessRead();
	}
	if (events & EPOLLOUT)
	{
		ProcessWrite();
	}
}

void	Session::Disconnect()
{
	if (ServiceRef service = _service.lock())
	{
		EpollCoreRef	epollCore = service->GetEpollCore();
		{//현재 이벤트 타입이 이미 disconnect면 다른스레드에서 disconnect 실행한거니까 그냥 종료
			std::lock_guard<std::mutex>	lock(m);
			if (_epollEvent->GetEventType() == EventType::Disconnect)
				return;
			ModEvent(EventType::Disconnect);
		}

		if (_sslObject)
		{
			int	ret = SSL_shutdown(_sslObject->GetSsl());
			if (ret != 1) 
			{// 정상|비정상 종료. 상대 클로즈노티 대기.하다가 세션 disconnect
				// 2초 디스커넥트 타이머
				TimerRef	timer = make_shared<Timer>(
					[self_weak = std::weak_ptr<Session>(shared_from_this())]()
					{
						if (std::shared_ptr<Session> session = self_weak.lock())
						{
							session->ProcessDisconnect(true);
						}
					},
					2,
					service
				);
				EpollEvent*	timerEvent = new EpollEvent(timer, EventType::Timer);
				timer->SetEpollEvent(timerEvent);
				epollCore->Register(timerEvent);
			}
		}
	}
}

void	Session::Send(WriteBufferRef writeBuffer)
{
	if (ServiceRef service = _service.lock())
	{
		std::lock_guard<std::mutex>	lock(m);
	
		_writeBuffers.push_back(writeBuffer);
		ModEvent(EventType::Write);
	}
}


void	Session::ProcessConnect()
{

}

void	Session::ProcessDisconnect(bool isCanSslShutdown)
{
	if (isCanSslShutdown)
	{
		SSL_shutdown(_sslObject->GetSsl());
	}
	_sslObject = nullptr;
	if (ServiceRef service = _service.lock())
	{
		EpollCoreRef	epollCore = service->GetEpollCore();
		if (_epollEvent)
		{
			service->EraseSession(_epollEvent->GetOwner());
			epollCore->DelEvent(_epollEvent);
			delete _epollEvent;
			_epollEvent = nullptr;
		}
	}
}

void	Session::ProcessRead()
{
	SSL*	ssl = _sslObject->GetSsl();
	do {
		int	readLen = SSL_read(ssl, _readBuffer.ReadPos(), READ_SIZE);
		if (readLen <= 0)
		{
			int err = SSL_get_error(ssl, readLen);
			if (err == SSL_ERROR_WANT_READ){//복호화 하기에 데이터 부족함.
				return;
			} else if (err == SSL_ERROR_ZERO_RETURN) {
				// 상대가 close 함. SSL_shutdown ok
				ProcessDisconnect(true);
				return;
			} else {//SSL에 심각한 에러 발생. SSL_shutdown 금지.
				ProcessDisconnect(false);
				return;
			}
		}
		if (_readBuffer.OnWrite(readLen) == false)
		{
			ProcessDisconnect(true);
			return;
		}
	} while (SSL_has_pending(ssl));
	//TODO 읽은내용 Protobuf로 역직렬화 해서 메세지 구조체 만들고, 잡큐에 넣기.
	int	processLen = OnRead(_readBuffer.ReadPos(), _readBuffer.DataSize());
	_readBuffer.OnRead(processLen);
	_readBuffer.Clean();
}

void	Session::ProcessWrite()
{
	SSL*	ssl = _sslObject->GetSsl();
	while (!_writeBuffers.empty())//writebuffers가 빌때까지 반복
	{
		WriteBufferRef	writeBuffer;
		BYTE*			buffer;
		int				dataLen;
		{
			std::lock_guard<std::mutex>	lock(m);

			writeBuffer = _writeBuffers.front();
			buffer = writeBuffer->GetBuffer();
			dataLen = writeBuffer->GetDataLen();
			_writeBuffers.pop_front();
		}
		
		size_t	writeLen = 0;
		int rtn = SSL_write_ex(ssl, buffer, dataLen, &writeLen);
		if (rtn == 0)
		{// 실패
			int err = SSL_get_error(ssl, rtn);
			if (err == SSL_ERROR_WANT_WRITE){//커널 버퍼 공간 부족
				return;
			} else if (err == SSL_ERROR_ZERO_RETURN) {
				//??
				ProcessDisconnect(true);
				return;
			} else {//SSL에 심각한 에러 발생. SSL_shutdown 금지.
				ProcessDisconnect(false);
				return;
			}
		}
		else if (writeLen != dataLen)
		{//일부만 write 성공 시
			if (writeBuffer->UpdateWritePos(writeLen) == false)
			{
				ProcessDisconnect(true);
				return;
			}
			std::lock_guard<std::mutex>	lock(m);
			_writeBuffers.push_front(writeBuffer);
		}
		ModEvent(EventType::Read);
	}
}

void	Session::ModEvent(EventType type)
{
	_epollEvent->SetEventType(type);
	if (ServiceRef service = _service.lock())
		service->GetEpollCore()->ModEvent(_epollEvent);	
}

/*====================
    PacketSession
====================*/


PacketSession::PacketSession(int clinetSocket, struct sockaddr_in addr, ServiceRef service)
	: Session(clinetSocket, addr, service) {}

PacketSession::~PacketSession() {}

int	PacketSession::OnRead(BYTE* buffer, int len)
{
	int	processLen = 0;
	while (true)
	{
		int	dataSize = len - processLen;

		if (dataSize < sizeof(PacketHeader))
			break;
		struct PacketHeader* header = reinterpret_cast<struct PacketHeader*>(&buffer[processLen]);
		
		if (dataSize < header->size)
			break;

		OnReadPacket(&buffer[processLen], header->size);
		
		processLen += header->size;
	}

	return processLen;
}
