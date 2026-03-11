#pragma once

#include <hiredis.h>
#include <string>

class RedisConnection
{
public:
	RedisConnection() {}
	~RedisConnection();

	bool	Connect(const char* ip, int port);
	void	Clear();

	bool	Execute(const std::string& query);
	
private:
	redisContext*	_connection = nullptr;
	redisReply*		_reply = nullptr;
};