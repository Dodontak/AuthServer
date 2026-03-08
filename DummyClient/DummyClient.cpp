
#include "Service.h"
#include "Types.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "ServerPacketHandler.h"
#include "DBConnectionPool.h"
#include "ServerSession.h"
#include <thread>
#include <libpq-fe.h>
#include <iostream>
#include <string>

using namespace std;

void	Broadcast(ClientServiceRef service)
{
	Protocol::C_SIGNUP	pkt;
	pkt.set_email("asd@naver.com");
	pkt.set_nickname("Dodontak");
	pkt.set_password("password123");
	WriteBufferRef	writeBuffer = ServerPacketHandler::MakeWriteBuffer(pkt);
	std::this_thread::sleep_for(chrono::seconds(1));
	service->broadcastfortest(writeBuffer);
}

void	WorkerThread()
{
	while (true)
	{
		JobRef job = nullptr;
		{
			std::unique_lock<std::mutex> lock(GThreadManager->_m);
			GThreadManager->_cv.wait(lock, []() { return !GJobQueue->Empty(); });
			job = GJobQueue->PopJob();
		}
		job->Execute();
	}
}

int main()
{
	ServerPacketHandler::Init();

	for (int i = 0; i < 10; i++)
		GThreadManager->Launch(WorkerThread);
	ClientServiceRef	service = make_shared<ClientService>(
		"127.0.0.1",
		4242,
		[](int clientSocket, sockaddr_in addr, ServiceRef service) {
        	return std::make_shared<ServerSession>(clientSocket, addr, service);
    	},
		10
	);
	GThreadManager->Launch(
		[service](){
			Broadcast(service);
		}
	);
	service->Start();

	GThreadManager->Join();
}
