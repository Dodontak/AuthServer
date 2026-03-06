#include "ThreadManager.h"
#include "CoreTLS.h"
#include <iostream>
#include <atomic>

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void	ThreadManager::InsertJob(std::function<void()> callback)
{
	JobRef	job = std::make_shared<Job>(callback);
	GJobQueue->PushJob(job);
	_cv.notify_one();
}

void	ThreadManager::InsertJob(JobRef	job)
{
	GJobQueue->PushJob(job);
	_cv.notify_one();
}

void	ThreadManager::Launch(std::function<void()> callback)
{
	_workerThreads.push_back(std::thread([=](){
		InitTLS();
		callback();
		DestroyTLS();
	}));
}

void	ThreadManager::Join()
{
	for (auto& t : _workerThreads)
	{
		t.join();
	}
}


void	ThreadManager::InitTLS()
{
	static std::atomic<int>	SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void	ThreadManager::DestroyTLS() {}