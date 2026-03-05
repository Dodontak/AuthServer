#pragma once

#include "JobQueue.h"
#include "CoreGlobal.h"
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();
	
	void	Launch();
	void	Join();
	void	WorkerThread();
	void	InsertJob(std::function<void()> callback);
	void	InsertJob(JobRef job);
private:
	static void	InitTLS();
	static void	DestroyTLS();

	std::mutex					_m;
	std::vector<std::thread>	_workerThreads;
	std::condition_variable		_cv;
};
