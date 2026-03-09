#include "PGConnection.h"

PGConnection::~PGConnection()
{
	PQfinish(_connection);
}

bool	PGConnection::Connect(const char* connectionString)
{
	_connection = PQconnectdb(connectionString);
	if (PQstatus(_connection) != CONNECTION_OK)
        return false;
	return true;
}


PGresult*	PGConnection::ExecuteSQL(const char* sql)
{
	return PQexec(_connection, sql);
}