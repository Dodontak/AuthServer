#include "ThreadManager.h"
#include "CoreTLS.h"
#include <iostream>
#include <atomic>

using namespace std;

ThreadManager::ThreadManager() : _running(true)
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void	ThreadManager::Launch(function<void()> callback)
{
	_workerThreads.push_back(thread([=](){
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
	static atomic<int>	SThreadId(1);
	LThreadId = SThreadId.fetch_add(1);
}

void	ThreadManager::DestroyTLS() {}