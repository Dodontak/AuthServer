#include "DBConnectionPool.h"

bool	DBConnectionPool::Connect(int connectionCount, const char* connectionString)
{
	_connections.reserve(connectionCount);
	for (int i = 0; i < connectionCount; i++)
	{
		DBConnectionRef	conn = make_shared<DBConnection>(connectionString);
		_connections.push_back(conn);
	}
	return true;
}

void	DBConnectionPool::Push(DBConnectionRef conn)
{
	std::lock_guard<std::mutex>	lock(_m);
	_connections.push_back(conn);
}


DBConnectionRef	DBConnectionPool::Pop()
{
	DBConnectionRef	connection = nullptr;
	std::lock_guard<std::mutex>	lock(_m);
	if (_connections.empty())
		return connection;
	connection = _connections.back();
	_connections.pop_back();
	return connection;
}