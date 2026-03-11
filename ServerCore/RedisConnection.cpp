#include "RedisConnection.h"
#include <iostream>

using namespace std;

RedisConnection::~RedisConnection()
{
	Clear();
	if(_connection)
		redisFree(_connection);
}

bool	RedisConnection::Connect(const char* ip, int port)
{
	_connection = redisConnect(ip, port);
	if (_connection == NULL || _connection->err) {
		if (_connection != NULL)
			cout << _connection->errstr << endl;
		else
			cout << "Can't allocate redis context" << endl;
		return false;
	}
	return true;
}

void	RedisConnection::Clear()
{
	if (_reply)
	{
		freeReplyObject(_reply);
		_reply = nullptr;
	}
}

bool	RedisConnection::IsReplyError()
{
	if (_reply == nullptr || _reply->type == REDIS_REPLY_ERROR) {
		if (_reply == nullptr)
			return true;
		else
			return true;
	}
	return false;
}
