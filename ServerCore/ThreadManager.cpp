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