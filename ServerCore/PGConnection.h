#pragma once

#include "Utils.h"
#include <libpq-fe.h>
#include <string>

class PGConnection
{
public:
	PGConnection() {}
	~PGConnection();

	bool		Connect(const char* connectionString);
	PGresult*	ExecuteSQL(const char* sql);
private:
	PGconn* 	_connection = nullptr;
};