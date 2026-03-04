#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"

ThreadManager*	GThreadManager = nullptr;
JobQueue*	GJobQueue = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GJobQueue = new JobQueue();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GJobQueue;
	}
}	GcoreGlobal;