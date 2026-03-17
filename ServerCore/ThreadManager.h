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

    bool    IsRunning() { return _running; }
	
	std::mutex				_workerMutex;
	std::condition_variable	_workerCv;

	std::mutex				_mailMutex;
	std::condition_variable	_mailCv;
private:
	static void	InitTLS();
	static void	DestroyTLS();
    bool        _running;

	std::vector<std::thread>		_workerThreads;
	std::vector<std::thread>		_mailThreads;
};
