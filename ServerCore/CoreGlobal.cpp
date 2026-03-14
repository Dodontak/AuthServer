#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "DBConnectionPool.h"
#include "SMTPConnection.h"

ThreadManager*	GThreadManager = nullptr;
JobQueue*	GJobQueue = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;
SMTPManager*	GSMTPManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GJobQueue = new JobQueue();
		GDBConnectionPool = new DBConnectionPool();
		GSMTPManager = new SMTPManager();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GJobQueue;
		delete GDBConnectionPool;
		delete GSMTPManager;
	}
}	GcoreGlobal;