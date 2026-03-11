#pragma once

#include "Utils.h"
#include <libpq-fe.h>
#include <string>
#include <vector>

class PGConnection
{
public:
	PGConnection() {}
	~PGConnection();

	bool		Connect(const char* connectionString);
	void		Clear();
	void		ClearValues();

	void		AddValue(const std::string& val);
	bool		ExecuteSQL(const std::string& sql);
	int			GetRowCount();
	std::string	GetValue(int row, int col);
	bool		IsNull(int row, int col);

private:
	PGconn* 			_connection = nullptr;
	PGresult*			_result = nullptr;
	vector<std::string>	_values;
};