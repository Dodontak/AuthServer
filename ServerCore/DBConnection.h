#pragma once

#include "Utils.h"
#include <libpq-fe.h>

class DBConnection
{
public:
	DBConnection(const char* connectionString);
	~DBConnection();

	PGresult*	ExecuteSQL(const char* sql);
private:
	PGconn* 	_connection = nullptr;
};