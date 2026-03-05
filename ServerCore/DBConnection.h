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

// PGresult* rst = PQexec(conn, "insert into auth.users (name, emailaddress, hashed_pw) values ('dodontak', 'abc@naver.com', 'abc12345');");
// 	PQresultStatus(rst)

// 내용	접근 함수
// 실행 상태	PQresultStatus(res)
// 에러 메시지	PQresultErrorMessage(res)
// row 개수	PQntuples(res)
// column 개수	PQnfields(res)
// 특정 row/col 값	PQgetvalue(res, row, col)
// column 이름	PQfname(res, col)