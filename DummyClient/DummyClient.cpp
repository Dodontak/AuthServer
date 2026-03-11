
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

void	SignUp(SessionRef session)
{
	Protocol::C_SIGNUP	pkt;
	std::string	email, nickname, password;
	email = GetTempId(20) + '@' + GetTempId(6) + ".com";
	pkt.set_email(email);
	nickname = GetTempId(20);
	pkt.set_nickname(nickname);
	password = "asdqwezxc123";//GetTempId(20); 테스트용 password
	pkt.set_password(password);
	WriteBufferRef	writeBuffer = ServerPacketHandler::MakeWriteBuffer(pkt);
	session->Send(writeBuffer);
}
//std::this_thread::sleep_for(chrono::seconds(1));

void	SignUpThread(ClientServiceRef service)
{
	std::this_thread::sleep_for(chrono::seconds(1));
	cout << "Start SignUpThread" << endl;
	for (int i = 0; i < 5; i++)
	{//랜덤하게 10개 세션 골라서 SignUp시킴
		SessionRef session = service->GetRandomSession();
		if (session)
			SignUp(session);
	}
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

int main(int ac, char** av)
{
	if (ac != 2)
	{
		cout << av[0] << " [port]" << endl;
		return 1;	
	}
	ServerPacketHandler::Init();

	for (int i = 0; i < 10; i++)
		GThreadManager->Launch(WorkerThread);
	ClientServiceRef	service = make_shared<ClientService>(
		"127.0.0.1",
		atoi(av[1]),
		[](int clientSocket, sockaddr_in addr, ServiceRef service) {
        	return std::make_shared<ServerSession>(clientSocket, addr, service);
    	},
		5
	);
	GThreadManager->Launch(
		[service](){
			SignUpThread(service);
		}
	);
	service->Start();

	GThreadManager->Join();
}
