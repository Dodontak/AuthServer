#include "ClientPacketHandler.h"
#include "CoreGlobal.h"
#include "DBConnectionPool.h"
#include "Types.h"
#include "Timer.h"
#include "bcrypt/BCrypt.hpp"
#include "SMTPConnection.h"
#include "ThreadManager.h"
#include <thread>
#include <string>
#include <libpq-fe.h>
#include <iomanip>
#include <openssl/rand.h>
#include <jwt-cpp/jwt.h>

using namespace std;

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

string CreateAccessToken(const string& user_id, const string& nickname)
{
    // 비밀키는 환경변수나 설정파일에서 읽어야 해요
    // 코드에 하드코딩 절대 금지
    const string SECRET_KEY = getenv("JWT_SECRET_KEY");

    auto token = jwt::create()
        .set_issuer("auth_server")          // 발급자
        .set_type("JWT")
        .set_payload_claim("user_id",  jwt::claim(user_id))
        .set_payload_claim("nickname", jwt::claim(nickname))
        .set_issued_at(chrono::system_clock::now())
        .set_expires_at(chrono::system_clock::now() 
                        + chrono::hours(1))    // 1시간
        .sign(jwt::algorithm::hs256{SECRET_KEY});   // 비밀키로 서명

    return token;
}

bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt)
{
	cout << "Handle_C_SIGNUP" << endl;
	Protocol::S_SIGNUP	response;
	string	nickname = pkt.nickname();
	string	email = pkt.email();
	bool	skip_email = pkt.skip_email();
	
	string	pgGetUserIdSQL = "SELECT user_id FROM auth.users WHERE nickname = $1 OR email = $2";
	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(nickname);
	pg->AddValue(email);
	if(pg->ExecuteSQL(pgGetUserIdSQL) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_SIGNUP" << endl;
		GDBConnectionPool->Push(&pg);
		return;
	}
	int	rowCount = pg->GetRowCount();
	pg->Clear();

	GDBConnectionPool->Push(&pg);

	if (rowCount == 0)// ID,email 중복 없음 생성가능
	{
		string	hashedPassword = BCrypt::generateHash(pkt.password(), 10);
		string	temp_id = GetTempId(32);
	
		string	redisSetNickname = "SET %s:nickname %s EX 300";
		string	redisSetPassword = "SET %s:password %s EX 300";
		string	redisSetEmail = "SET %s:email %s EX 300";

		RedisConnection* redis = GDBConnectionPool->PopRedis();

		if (false == redis->Execute(redisSetNickname, temp_id.c_str(), nickname.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
			return;
		}
		redis->Clear();

		if (false == redis->Execute(redisSetPassword, temp_id.c_str(), hashedPassword.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
			return;
		}
		redis->Clear();

		if (false == redis->Execute(redisSetEmail, temp_id.c_str(), email.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
			return;
		}
		redis->Clear();
		GDBConnectionPool->Push(&redis);

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
	cout << "Handle_C_VERIFY_MAIL_REQ" << endl;
	Protocol::S_VERIFY_MAIL_REQ	response;

	string	temp_id = pkt.temp_id();//TODO 임시id 유효성 검사
	string	verfiy_code = "12341234";//GetTempId(8); 더미클라이언트 테스트용 고정코드


	string	redisGetEmail = "GET %s:email";
	string	redisSetVerifyCode = "SET %s:verify_code %s EX 180";

	RedisConnection* redis = GDBConnectionPool->PopRedis();

	if (false == redis->Execute(redisGetEmail, temp_id.c_str()))//아까 등록한 이메일 레디스에서 가져오기
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}
	string	a1_email = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisSetVerifyCode, temp_id.c_str(), verfiy_code.c_str()))//인증코드 레디스에 저장
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}
	string	a2_status = redis->GetStr();
	redis->Clear();

	GDBConnectionPool->Push(&redis);
	//TODO EmailAPI로 이메일 보내기, 실패시 처리
	GSMTPManager->PushMail(make_shared<Mail>(a1_email, "Verify Code From AuthServer", verfiy_code));
	GThreadManager->_mailCv.notify_one();
	
	response.set_success(true);
	response.set_temp_id(temp_id);
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::C_VERIFY_EMAIL_CODE& pkt)
{
	cout << "Handle_C_VERIFY_EMAIL_CODE" << endl;
	Protocol::S_VERIFY_EMAIL_CODE	response;

	string	temp_id = pkt.temp_id();//TODO 임시id 유효성 검사
	string	verify_code = pkt.verify_code();
	
	string	redisGetVerifyCode = "GET %s:verify_code";
	string	redisGetNickname = "GET %s:nickname";
	string	redisGetPassword = "GET %s:password";
	string	redisGetEmail = "GET %s:email";

	RedisConnection* redis = GDBConnectionPool->PopRedis();

	if (false == redis->Execute(redisGetVerifyCode, temp_id.c_str()))
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}

	if (redis->IsNull() == REDIS_REPLY_NIL)
	{//인증코드가 만료됐을 경우
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		response.set_success(false);
		response.set_expired(true);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a1_verify_code = redis->GetStr();
	redis->Clear();

	if (verify_code != a1_verify_code)
	{//인증코드가 틀렸을 경우
		GDBConnectionPool->Push(&redis);
		response.set_success(false);
		response.set_expired(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	/* 인증코드가 맞았을 경우 */
	if (false == redis->Execute(redisGetNickname, temp_id.c_str()))
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}
	string	a2_nickname = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisGetPassword, temp_id.c_str()))
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}
	string	a3_password = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisGetEmail, temp_id.c_str()))
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
		return;
	}
	string	a4_email = redis->GetStr();
	redis->Clear();
	
	GDBConnectionPool->Push(&redis);

	string	pgSetUserDataSQL = "INSERT INTO auth.users (nickname, password, email) VALUES ($1, $2, $3)";
	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(a2_nickname);
	pg->AddValue(a3_password);
	pg->AddValue(a4_email);

	if(pg->ExecuteSQL(pgSetUserDataSQL) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_VERIFY_EMAIL_CODE" << endl;
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		return;
	}
	pg->Clear();
	GDBConnectionPool->Push(&pg);

	response.set_success(true);
	response.set_expired(false);
	response.set_nickname(a2_nickname);
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	cout << "Handle_C_LOGIN" << endl;
	Protocol::S_LOGIN	response;
	string			nickname = pkt.nickname();
	string			password = pkt.password();

	string		pgGetUserData = "SELECT user_id, password, is_block FROM auth.users WHERE nickname = $1";
	PGConnection*	pg = GDBConnectionPool->PopPG();
	
	pg->AddValue(nickname);

	if(pg->ExecuteSQL(pgGetUserData) == false)
	{//TODO 실행실패 제대로 처리
		cout << "Fail To ExecuteSQL in Handle_C_LOGIN" << endl;
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		return;
	}

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
	
	GDBConnectionPool->Push(&pg);

	if (BCrypt::validatePassword(password, a_password))
	{//로그인 성공
		string	token = CreateAccessToken(a_user_id, nickname);
		response.set_success(true);
		response.set_token(token);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//비밀번호 틀림
		string	redisIncrFailCount = "INCR %s:fail_count EX 600";
		RedisConnection*	redis = GDBConnectionPool->PopRedis();
		if(false == redis->Execute(redisIncrFailCount, nickname.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
			return;
		}
		int	fail_count = redis->GetInt();
		redis->Clear();
		if (fail_count >= 5)
		{
			//TODO 차단
		}
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
}