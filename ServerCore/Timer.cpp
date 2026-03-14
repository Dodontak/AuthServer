#include "Timer.h"
#include "EpollCore.h"

using namespace std;

Timer::Timer(function<void()> callback, int sec, ServiceRef service) : _callback(callback), _service(service)
{
	_tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	_ts.it_value.tv_sec = sec;
	_ts.it_value.tv_nsec = 0;
	cout << "Timer " << _tfd << " constructed." << endl;
	timerfd_settime(_tfd, 0, &_ts, nullptr);
}

Timer::~Timer()
{
	cout << "Timer " << _tfd << " distructed." << endl;
}

void	Timer::Dispatch(uint32_t events)
{
	uint64	expirations;
	int		s = read(_tfd, &expirations, sizeof(expirations));
	if (s != sizeof(expirations))
	{
		ReleaseTimer();
		return;
	}
	_callback();
	ReleaseTimer();
}

void	Timer::ReleaseTimer()
{
	if (ServiceRef service = _service.lock())
	{
		service->GetEpollCore()->DelEvent(_timerEvent);
	}
	if (_timerEvent)
	{
		delete _timerEvent;
		_timerEvent = nullptr;
	}
	close(_tfd);
}