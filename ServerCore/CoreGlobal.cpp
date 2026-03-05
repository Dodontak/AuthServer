#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "DBConnectionPool.h"

ThreadManager*	GThreadManager = nullptr;
JobQueue*	GJobQueue = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GJobQueue = new JobQueue();
		GDBConnectionPool = new DBConnectionPool();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GJobQueue;
		delete GDBConnectionPool;
	}
}	GcoreGlobal;