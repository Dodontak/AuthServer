#pragma once

#include <hiredis.h>
#include <string>

class RedisConnection
{
public:
	RedisConnection() {}
	~RedisConnection();

	bool		Connect(const char* ip, int port);
	redisReply*	Execute(std::string query);
private:
	redisContext*	_connection = nullptr;
};