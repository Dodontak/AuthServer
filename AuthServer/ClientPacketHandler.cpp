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

using namespace std;

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

// string CreateAccessToken(const string& user_id, const string& nickname)
// {
//     // 비밀키는 환경변수나 설정파일에서 읽어야 해요
//     // 코드에 하드코딩 절대 금지
//     const string SECRET_KEY = getenv("JWT_SECRET_KEY");

//     auto token = jwt::create()
//         .set_issuer("auth_server")          // 발급자
//         .set_type("JWT")
//         .set_payload_claim("user_id",  jwt::claim(user_id))
//         .set_payload_claim("nickname", jwt::claim(nickname))
//         .set_issued_at(chrono::system_clock::now())
//         .set_expires_at(chrono::system_clock::now() 
//                         + chrono::hours(1))    // 1시간
//         .sign(jwt::algorithm::hs256{SECRET_KEY});   // 비밀키로 서명

//     return token;
// }

bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt)
{
	Protocol::S_SIGNUP	response;
	string nickname = pkt.nickname();
	string email = pkt.email();
	bool		skip_email = pkt.skip_email();
	
	string	pgGetUserIdSQL = "SELECT user_id FROM auth.users WHERE nickname = $1 OR email = $2";
	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(nickname);
	pg->AddValue(email);
	if(pg->ExecuteSQL(pgGetUserIdSQL) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_SIGNUP" << endl;
		GDBConnectionPool->Push(pg);
		return;
	}
	GDBConnectionPool->Push(pg);

	int	rowCount = pg->GetRowCount();
	pg->Clear();

	if (rowCount == 0)// ID,email 중복 없음 생성가능
	{
		string hashedPassword = BCrypt::generateHash(pkt.password(), 10);
		string	temp_id = GetTempId(32);
		string	base = "SET " + temp_id;
		string	q1, q2, q3, a1, a2, a3;
	
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
	string	temp_id = pkt.temp_id();
	//TODO 임시id 유효성 검사
	string	verfiy_code = "12341234";//GetTempId(8); 더미클라이언트 테스트용 고정코드

	string	q1, q2;
	q1 = "GET " + temp_id + ":email";
	q2 = "SET " + temp_id + ":verify_code " + verfiy_code;
 	string	a1_email, a2_status;
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
	string	temp_id = pkt.temp_id();
	//TODO 임시id 유효성 검사
	string	verify_code = pkt.verify_code();

	string	getBase = "GET " + temp_id;
	string	q1_verify_code, q2_nickname, q3_password, q4_email;
	string	a1_verify_code, a2_nickname, a3_password, a4_email;
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

	string	pgSetUserDataSQL = "INSERT INTO auth.users (nickname, password, email) VALUES ($1, $2, $3)";
	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(a2_nickname);
	pg->AddValue(a3_password);
	pg->AddValue(a4_email);

	if(pg->ExecuteSQL(pgSetUserDataSQL) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_VERIFY_EMAIL_CODE" << endl;
		pg->Clear();
		GDBConnectionPool->Push(pg);
		return;
	}
	GDBConnectionPool->Push(pg);
	pg->Clear();

	response.set_success(true);
	response.set_expired(false);
	response.set_nickname(a2_nickname);
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	Protocol::S_LOGIN	response;
	string			nickname = pkt.nickname();
	string			password = pkt.password();

	string		pgGetPassword = "SELECT user_id, password FROM auth.users WHERE nickname = $1";
	PGConnection*	pg = GDBConnectionPool->PopPG();
	
	pg->AddValue(nickname);

	if(pg->ExecuteSQL(pgGetPassword) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_LOGIN" << endl;
		pg->Clear();
		GDBConnectionPool->Push(pg);
		return;
	}

	GDBConnectionPool->Push(pg);

	if (pg->GetRowCount() == 0)
	{//존재하지 않는 nickname
		pg->Clear();
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	string	a_user_id = pg->GetValue(0, 0);
	string	a_password = pg->GetValue(0, 1);
	pg->Clear();

	if (BCrypt::validatePassword(password, a_password))
	{//로그인 성공
		string	token = "token";//CreateAccessToken(a_user_id, nickname);
		response.set_success(true);
		response.set_token(token);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//비밀번호 틀림
		string	query = "INCR " + nickname + ":fail_count EX 600";
		RedisConnection*	redis = GDBConnectionPool->PopRedis();
		redisReply*	reply = redis->Execute(query);
		int	fail_count = reply->integer;
		freeReplyObject(reply);
		GDBConnectionPool->Push(redis);
		if (fail_count >= 5)
		{
			//TODO 차단
		}
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
}