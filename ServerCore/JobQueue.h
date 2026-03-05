#pragma once

#include "Types.h"
#include <queue>
#include <mutex>
#include <functional>

class Job
{
public:
	Job(std::function<void()> callback);
	
	void	Execute();
private:
	std::function<void()>	_callback;
};

class JobQueue
{
public:
	void	PushJob(JobRef job);
	JobRef	PopJob();
	bool	Empty() { return _jobQueue.empty(); }
private:
	std::mutex		m;
	std::queue<JobRef>	_jobQueue;
};