#include "DBConnection.h"

DBConnection::DBConnection(const char* connectionString)
{
	_connection = PQconnectdb(connectionString);
	if (PQstatus(_connection) != CONNECTION_OK)
        handle_error("PQconnectdb error", 1);
}

DBConnection::~DBConnection()
{
	PQfinish(_connection);
}


PGresult*	DBConnection::ExecuteSQL(const char* sql)
{
	return PQexec(_connection, sql);
}