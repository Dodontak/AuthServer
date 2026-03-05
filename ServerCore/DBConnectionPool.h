#pragma once

#include "Types.h"
#include "DBConnection.h"
#include <vector>
#include <mutex>

class DBConnectionPool
{
public:
	DBConnectionPool() {}
	~DBConnectionPool() {}

	bool			Connect(int connectionCount, const char* connectionString);
	void			Push(DBConnectionRef conn);
	DBConnectionRef	Pop();
private:
	std::mutex						_m;
	std::vector<DBConnectionRef>	_connections;
};
