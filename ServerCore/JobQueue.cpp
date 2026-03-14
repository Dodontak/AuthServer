#include "JobQueue.h"
#include "CoreTLS.h"
#include <iostream>

using namespace std;

/*=======================
		  Job
=======================*/

Job::Job(function<void()> callback) : _callback(callback) {}

void	Job::Execute()
{
	_callback();
}

/*=======================
		JobQueue
=======================*/

void	JobQueue::PushJob(JobRef job)
{
	lock_guard<mutex>	lock(m);
	_jobQueue.push(job);
}

JobRef	JobQueue::PopJob()
{
	JobRef	job = nullptr;
	lock_guard<mutex>	lock(m);

	if (_jobQueue.empty())
		return job;
	job = _jobQueue.front();
	_jobQueue.pop();
	return job;
}