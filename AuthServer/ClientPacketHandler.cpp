#include "ClientPacketHandler.h"
#include "CoreGlobal.h"
#include "DBConnectionPool.h"
#include "Types.h"
#include "Timer.h"
#include "bcrypt/BCrypt.hpp"
#include <thread>
#include <string>
#include <libpq-fe.h>
#include <iomanip>
#include <openssl/rand.h>

std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt)
{
	Protocol::S_SIGNUP	response;
	std::string nickname = pkt.nickname();
	std::string email = pkt.email();
	bool		skip_email = pkt.skip_email();
	
	std::string	pgsql = "SELECT user_id FROM auth.users WHERE nickname = '" + nickname + "' OR email = '" + email + "'";
	//TODO 커넥션풀에 남은거 없을 때 처리
	PGConnection*	pg = GDBConnectionPool->PopPG();
	PGresult*	pgResult = pg->ExecuteSQL(pgsql.c_str());
	GDBConnectionPool->Push(pg);
	int	num = PQntuples(pgResult);
	if (num == 0)// ID,email 중복 없음 생성가능
	{
		std::string hashedPassword = BCrypt::generateHash(pkt.password());
		std::string	temp_id = GetTempId(32);
		std::string	base = "SET " + temp_id;
		std::string	q1, q2, q3, a1, a2, a3;
	
		q1 = base + ":nickname " + nickname;
		q2 = base + ":password " + hashedPassword;
		q3 = base + ":email " + email;

		redisReply *reply;
		RedisConnection* redis = GDBConnectionPool->PopRedis();
		//TODO 커넥션풀에 남은거 없을 때 처리
		reply = redis->Execute(q1.c_str());//set 닉네임
		if (RedisConnection::isReplyError(reply))
		{//에러면 함수안에서 free해줌.
			GDBConnectionPool->Push(redis);
			return;
		}
		a1 = reply->str;
		freeReplyObject(reply);

		reply = redis->Execute(q2.c_str());//set password
		if (RedisConnection::isReplyError(reply))
		{//에러면 함수안에서 free해줌.
			GDBConnectionPool->Push(redis);
			return;
		}
		a2 = reply->str;
		freeReplyObject(reply);

		reply = redis->Execute(q3.c_str());//set email
		if (RedisConnection::isReplyError(reply))
		{//에러면 함수안에서 free해줌.
			GDBConnectionPool->Push(redis);
			return;
		}
		a3 = reply->str;
		freeReplyObject(reply);

		GDBConnectionPool->Push(redis);
		redis = nullptr;
		if (a1 != "OK" || a2 != "OK" || a3 != "OK")
		{//실패 있을경우 에러처리
			return ;
		}
		response.set_success(true);
		response.set_skip_email(skip_email);
		response.set_temp_id(temp_id);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else// ID,email 중복 있음 생성 불가능
	{
		response.set_success(false);
		response.set_skip_email(skip_email);
		response.set_temp_id("");
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
}

void	Handle_C_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::C_VERIFY_MAIL_REQ& pkt)
{
	Protocol::S_VERIFY_MAIL_REQ	response;
	std::string	temp_id = pkt.temp_id();
	//TODO 임시id 유효성 검사
	std::string	verfiy_code = "12341234";//GetTempId(8); 더미클라이언트 테스트용 고정코드

	std::string	q1, q2;
	q1 = "GET " + temp_id + ":email";
	q2 = "SET " + temp_id + ":verify_code " + verfiy_code;
 	std::string	a1_email, a2_status;
	redisReply *reply;
	RedisConnection* redis = GDBConnectionPool->PopRedis();

	reply = redis->Execute(q1);//아까 등록한 이메일 레디스에서 가져오기
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	a1_email = reply->str;
	freeReplyObject(reply);

	reply = redis->Execute(q2);//인증코드 레디스에 저장
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	a2_status = reply->str;
	freeReplyObject(reply);

	GDBConnectionPool->Push(redis);
	//TODO EmailAPI로 이메일 보내기, 실패시 처리
	response.set_success(true);
	response.set_temp_id(temp_id);
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::C_VERIFY_EMAIL_CODE& pkt)
{
	Protocol::S_VERIFY_EMAIL_CODE	response;
	std::string	temp_id = pkt.temp_id();
	//TODO 임시id 유효성 검사
	std::string	verify_code = pkt.verify_code();

	std::string	getBase = "GET " + temp_id;
	std::string	q1_verify_code, q2_nickname, q3_password, q4_email;
	std::string	a1_verify_code, a2_nickname, a3_password, a4_email;
	q1_verify_code =  getBase + ":verify_code";
	q2_nickname =  getBase + ":nickname";
	q3_password =  getBase + ":password";
	q4_email =  getBase + ":email";

	redisReply *reply;
	RedisConnection* redis = GDBConnectionPool->PopRedis();

	reply = redis->Execute(q1_verify_code);//get 확인용 인증코드
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	if (reply->type == REDIS_REPLY_NIL)
	{//인증코드가 만료됐을 경우
		freeReplyObject(reply);
		GDBConnectionPool->Push(redis);
		response.set_success(false);
		response.set_expired(true);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	a1_verify_code = reply->str;
	freeReplyObject(reply);

	if (verify_code != a1_verify_code)
	{//인증코드가 틀렸을 경우
		GDBConnectionPool->Push(redis);
		response.set_success(false);
		response.set_expired(true);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	/* 인증코드가 맞았을 경우 */
	reply = redis->Execute(q2_nickname);//get 레디스에 저장해둔 닉네임
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	a2_nickname = reply->str;
	freeReplyObject(reply);

	reply = redis->Execute(q3_password);//get 레디스에 저장해둔 비밀번호
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	a3_password = reply->str;
	freeReplyObject(reply);

	reply = redis->Execute(q4_email);//get 레디스에 저장해둔 비밀번호
	if (RedisConnection::isReplyError(reply))
	{//에러면 함수안에서 free해줌.
		GDBConnectionPool->Push(redis);
		return;
	}
	a4_email = reply->str;
	freeReplyObject(reply);
	
	GDBConnectionPool->Push(redis);

	std::string	pgsql = "INSERT INTO auth.users (nickname, password, email) VALUES \
		('" + a2_nickname + "', '" + a3_password + "', '" + a4_email + "')";
	PGConnection*	pg = GDBConnectionPool->PopPG();
	PGresult*	pgResult = pg->ExecuteSQL(pgsql.c_str());
	GDBConnectionPool->Push(pg);
	PQclear(pgResult);
	response.set_success(true);
	response.set_expired(false);
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	Protocol::S_LOGIN	response;
}