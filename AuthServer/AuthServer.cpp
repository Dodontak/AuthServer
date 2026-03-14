
#include "Service.h"
#include "Types.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "ClientPacketHandler.h"
#include "DBConnectionPool.h"
#include "CAuthSession.h"
#include "SMTPConnection.h"
#include <thread>
#include <libpq-fe.h>
#include <iostream>
#include <string>
#include <hiredis.h>
#include <openssl/rand.h>
#include <iomanip>

using namespace std;

void	WorkerThread()
{
	while (true)
	{
		JobRef job = nullptr;
		{
			unique_lock<mutex> lock(GThreadManager->_workerMutex);
			GThreadManager->_workerCv.wait(lock, []() { return !GJobQueue->Empty(); });
			job = GJobQueue->PopJob();
		}
		if (job)
			job->Execute();
	}
}

void	MailThread()
{
	while (true)
	{
		shared_ptr<Mail> mail = nullptr;
		{
			unique_lock<mutex> lock(GThreadManager->_mailMutex);
			GThreadManager->_mailCv.wait(lock, []() { return !GSMTPManager->Empty(); });
			mail = GSMTPManager->PopMail();
		}
		if (mail)
		{
			GSMTPManager->GetConnection()->SendMail(mail);
		}
	}
}

int main(int ac, char** av)
{
	if (ac != 2)
		handle_error((string(av[0]) + " [port]").c_str(), 1);
		GDBConnectionPool->Init(10, "redis", 6379, 10,
			"host=postgres user=postgres port=5432 dbname=postgres password=password connect_timeout=3");
	ClientPacketHandler::Init();
	GSMTPManager->Init("dodontak2@gmail.com");
	for (int i = 0; i < 5; i++)
		GThreadManager->Launch(WorkerThread);
	for (int i = 0; i < 3; i++)
		GThreadManager->Launch(MailThread);

	ServiceRef	service = make_shared<AuthService>(
		"127.0.0.1",
		stoi(av[1]),
		"TLS/server.crt",
		"TLS/server.key",
		[](int clientSocket, sockaddr_in addr, ServiceRef service) {
        	return make_shared<CAuthSession>(clientSocket, addr, service);
    	}
	);
	service->Start();
	GThreadManager->Join();
}
