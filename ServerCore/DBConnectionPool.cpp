#include "DBConnectionPool.h"

using namespace std;

bool	DBConnectionPool::Init(int maxRedis, const char* redisIp, int redisPort,
				int maxPostgres, const char* pgConString)
{
	_maxRedis = maxRedis;
	_maxPostgres = maxPostgres;
	_pgConString = pgConString;
	_redisIp = redisIp;
	_redisPort = redisPort;

	_postgresConnections.reserve(maxPostgres);
	for (int i = 0; i < _maxPostgres; ++i)
	{
		PGConnection*	conn = new PGConnection();
		if (conn->Connect(_pgConString.c_str()) == false)
			Utils::ErrorExit("PGConnect Error");
		_postgresConnections.push_back(conn);
	}

	_redisConnections.reserve(maxRedis);
	for (int i = 0; i < _maxRedis; ++i)
	{
		RedisConnection*	conn = new RedisConnection();
		if (conn->Connect(_redisIp.c_str(), _redisPort) == false)
			Utils::ErrorExit("RedisConnect Error");
		_redisConnections.push_back(conn);
	}
	return true;
}

void	DBConnectionPool::Push(PGConnection** conn)
{
	lock_guard<mutex>	lock(_mPostgres);
	_postgresConnections.push_back(*conn);
	*conn = nullptr;
}

void	DBConnectionPool::Push(RedisConnection** conn)
{
	lock_guard<mutex>	lock(_mRedis);
	_redisConnections.push_back(*conn);
	*conn = nullptr;
}


PGConnection*	DBConnectionPool::PopPG()
{
	PGConnection*	connection;
	lock_guard<mutex>	lock(_mPostgres);
	if (_postgresConnections.empty())
		return nullptr;
	connection = _postgresConnections.back();
	_postgresConnections.pop_back();
	return connection;
}

RedisConnection*	DBConnectionPool::PopRedis()
{
	RedisConnection*	connection;
	lock_guard<mutex>	lock(_mRedis);
	if (_redisConnections.empty())
		return nullptr;
	connection = _redisConnections.back();
	_redisConnections.pop_back();
	return connection;
}