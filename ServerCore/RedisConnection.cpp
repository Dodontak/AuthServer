#include "RedisConnection.h"
#include <iostream>

RedisConnection::~RedisConnection()
{
	if(_connection)
		redisFree(_connection);
}

bool	RedisConnection::Connect(const char* ip, int port)
{
	_connection = redisConnect(ip, port);
	if (_connection == NULL || _connection->err) {
		if (_connection != NULL)
			std::cout << _connection->errstr << '\n';
		else
			std::cout << "Can't allocate redis context\n";
		return false;
	}
	return true;
}

redisReply*	RedisConnection::Execute(const std::string& query)
{
	return	(redisReply *)redisCommand(_connection, query.c_str());
}

bool	RedisConnection::isReplyError(redisReply* reply)
{
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		if (reply == nullptr)
			return true;
		else
			freeReplyObject(reply);
			return true;
	}
	return false;
}
