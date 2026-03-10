#pragma once

#include <hiredis.h>
#include <string>

class RedisConnection
{
public:
	RedisConnection() {}
	~RedisConnection();

	bool		Connect(const char* ip, int port);
	redisReply*	Execute(const std::string& query);

	static	bool	isReplyError(redisReply* reply);
private:
	redisContext*	_connection = nullptr;
};