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
	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == -1)
		Utils::ErrorExit("socket error");

	if (false == SocketUtils::MakeSocketNonblock(_listenSocket))
        Utils::ErrorExit("Listener MakeSocketNonblock error");
    
    if (false == SocketUtils::SetReuseAddress(_listenSocket, true))
        Utils::ErrorExit("Listener SetReuseAddress error");

    if (false == SocketUtils::Bind(_listenSocket, _address))
        Utils::ErrorExit("Listener Bind error");

	if (listen(_listenSocket, LISTEN_BACKLOG) == -1)
		Utils::ErrorExit("listen error");

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
		Utils::ErrorExit("accept error");
	}
    
	if (SocketUtils::MakeSocketNonblock(clientSocket) == false)
    {
        SocketUtils::CloseSocket(clientSocket);
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
		Utils::ErrorExit("Listener Dispatch events error");
	Accept();
}
