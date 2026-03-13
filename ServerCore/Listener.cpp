#include "Listener.h"

#include "Service.h"
#include "Utils.h"
#include "EpollEvent.h"
#include "Session.h"
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

Listener::Listener(ServiceRef service, int port) : _service(service)
{
	_listenSocket = SocketUtil::CreateSocket();
	if (_listenSocket == -1)
		handle_error("socket error", 1);

	SocketUtil::MakeSocketNonblock(_listenSocket);
	struct sockaddr_in sock_addr = service->GetNetAddress().GetAddr();

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_listenSocket, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
		handle_error("bind error", 1);

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
	SocketUtil::MakeSocketNonblock(clientSocket);

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
