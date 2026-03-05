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

void	ThreadManager::WorkerThread()
{
	while (true)
	{
		JobRef job = nullptr;
		{
			std::unique_lock<std::mutex> lock(_m);
			_cv.wait(lock, [this]() { return !GJobQueue->Empty(); });
			job = GJobQueue->PopJob();
		}
		std::cout << "thread No." << LThreadId << " Awaken!" << std::endl;
		job->Execute();
	}
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

void	ThreadManager::Launch()
{
	_workerThreads.push_back(std::thread([=](){
		InitTLS();
		WorkerThread();
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