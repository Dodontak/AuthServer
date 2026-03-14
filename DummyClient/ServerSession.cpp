#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include <functional>
#include <iostream>

using namespace std;

ServerSession::ServerSession(int clientSocket, struct sockaddr_in addr, ServiceRef service)
	: PacketSession(clientSocket, addr, service)
{
	_isClient = false;
}


ServerSession::~ServerSession()
{

}


void	ServerSession::OnWrite(int len)
{
}

void	ServerSession::OnReadPacket(BYTE* buffer, int len)
{
	function<void()>	callback;
	PacketSessionRef	session = static_pointer_cast<PacketSession>(shared_from_this());
	bool	success = ServerPacketHandler::PacketHandler(callback, session, buffer, len);
	if (success)
	{
		JobRef	job = make_shared<Job>(callback);
		
		GJobQueue->PushJob(job);
		GThreadManager->_workerCv.notify_one();
	}
	else
	{
		Disconnect();
	}
}