#include "CAuthSession.h"
#include "ClientPacketHandler.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include <functional>
#include <iostream>

using namespace std;

CAuthSession::CAuthSession(int clientSocket, struct sockaddr_in addr, ServiceRef service)
	: PacketSession(clientSocket, addr, service)
{
	_isClient = true;
}

CAuthSession::~CAuthSession()
{

}

void	CAuthSession::OnWrite(int len)
{
}

void	CAuthSession::OnReadPacket(BYTE* buffer, int len)
{
	function<void()>	callback;
	PacketSessionRef	session = static_pointer_cast<PacketSession>(shared_from_this());
	bool result = ClientPacketHandler::PacketHandler(callback, session, buffer, len);
	if (result == false)
		return;
	else
	{
		JobRef	job = make_shared<Job>(callback);
		GJobQueue->PushJob(job);
		GThreadManager->_workerCv.notify_one();
	}
}