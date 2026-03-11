#pragma once

#include "Types.h"
#include "PGConnection.h"
#include "RedisConnection.h"
#include <vector>
#include <mutex>
#include <string>

class DBConnectionPool
{
public:
	DBConnectionPool() {}
	~DBConnectionPool() {}

	bool	Init(int maxRedis, const char* redisIp, int redisPort,
					int maxPostgres, const char* pgConString);

	void	Push(PGConnection** conn);
	void	Push(RedisConnection** conn);


	PGConnection*		PopPG();
	RedisConnection*	PopRedis();
private:
	std::mutex						_mPostgres;
	int								_maxPostgres;
	std::string						_pgConString;

	std::mutex						_mRedis;
	int								_maxRedis;
	std::string						_redisIp;
	int								_redisPort;

	std::vector<PGConnection*>		_postgresConnections;
	std::vector<RedisConnection*>	_redisConnections;
};
