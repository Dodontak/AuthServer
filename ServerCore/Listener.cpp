#include "Listener.h"

#include "NetAddress.h"
#include "Service.h"
#include "Utils.h"
#include "EpollEvent.h"
#include "Session.h"
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

Listener::Listener(ServiceRef service) : _service(move(service)), _address(service->GetNetAddress())
{
	_listenSocket = SocketUtil::CreateSocket();
	if (_listenSocket == -1)
		handle_error("socket error", 1);

	if (false == SocketUtil::MakeSocketNonblock(_listenSocket))
        handle_error("Listener MakeSocketNonblock error", 1);
    
    if (false == SocketUtil::SetReuseAddress(_listenSocket, true))
        handle_error("Listener SetReuseAddress error", 1);

    if (false == SocketUtil::Bind(_listenSocket, _address))
        handle_error("Listener Bind error", 1);

	if (listen(_listenSocket, LISTEN_BACKLOG) == -1)
		handle_error("listen error", 1);

	cout << "Listener constructed." << endl;
}

Listener::~Listener()
{
	close(_listenSocket);
}

void	Listener::Accept()
{
	struct sockaddr_in	client_addr;
	socklen_t			sock_len = sizeof(struct sockaddr_in);
	int	clientSocket = accept(_listenSocket, (struct sockaddr*)&client_addr, &sock_len);
	if (clientSocket == -1)
	{
		if (errno == EAGAIN)
			return;
		handle_error("accept error", 1);
	}
    
	if (SocketUtil::MakeSocketNonblock(clientSocket) == false)
    {
        SocketUtil::CloseSocket(clientSocket);
        return;
    }

	SessionRef		session = _service->MakeSession(clientSocket, client_addr, _service);
	EpollEvent*		epollEvent = new EpollEvent(session, EventType::HandShakingRead);

	_service->GetEpollCore()->Register(epollEvent);
	_service->InsertSession(session);
}

void	Listener::Dispatch(uint32_t events)
{
	if (events != EPOLLIN)
		handle_error("Listener Dispatch events error", 1);
	Accept();
}
