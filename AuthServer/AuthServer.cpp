
#include "Service.h"
#include "Types.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "ClientPacketHandler.h"
#include "DBConnectionPool.h"
#include "CAuthSession.h"
#include <thread>
#include <libpq-fe.h>
#include <iostream>
#include <string>

using namespace std;

int main(int ac, char** av)
{
	if (ac != 2)
		handle_error((string(av[0]) + " [port]").c_str(), 1);
	
	GDBConnectionPool->Connect(10,
		"host=postgres user=postgres port=5432 dbname=postgres password=password connect_timeout=3");
	ClientPacketHandler::Init();

	for (int i = 0; i < 5; i++)
		GThreadManager->Launch();
	ServiceRef	service = make_shared<Service>(
		"127.0.0.1",
		stoi(av[1]),
		"TLS/server.crt",
		"TLS/server.key",
		[](int clientSocket, sockaddr_in addr, ServiceRef service) {
        	return std::make_shared<CAuthSession>(clientSocket, addr, service);
    	}
	);
	service->Start();
	GThreadManager->Join();
}
