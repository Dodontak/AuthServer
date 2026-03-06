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
	
	void	Launch(std::function<void()> callback);
	void	Join();
	void	InsertJob(std::function<void()> callback);
	void	InsertJob(JobRef job);
	
	std::mutex				_m;
	std::condition_variable	_cv;
private:
	static void	InitTLS();
	static void	DestroyTLS();

	std::vector<std::thread>		_workerThreads;
};
