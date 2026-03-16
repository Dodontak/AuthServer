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

using namespace std;

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

bool    SignupToPG(string nickname, string pw, string email, bool skip_email)
{
    string	pgSetUserDataSQL;
    if (!skip_email)
        pgSetUserDataSQL = "INSERT INTO auth.users (nickname, password, email) VALUES ($1, $2, $3)";
    else 
        pgSetUserDataSQL = "INSERT INTO auth.users (nickname, password) VALUES ($1, $2)";
	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(nickname);
	pg->AddValue(pw);
    if (!skip_email)
	    pg->AddValue(email);

	if(pg->ExecuteSQL(pgSetUserDataSQL) == false)
	{
		cout << "Fail To ExecuteSQL in Handle_C_VERIFY_EMAIL_CODE" << endl;
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		return false;
	}
	pg->Clear();
	GDBConnectionPool->Push(&pg);
    return true;
}

void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt)
{
	cout << "Handle_C_SIGNUP" << endl;
	Protocol::S_SIGNUP	response;
    response.set_success(false);
	string	nickname = pkt.nickname();
	string	email = pkt.email();
	bool	skip_email = pkt.skip_email();
    response.set_skip_email(skip_email);
	
	string	pgGetUserIdSQL;
    if (!skip_email)
        pgGetUserIdSQL = "SELECT user_id FROM auth.users WHERE nickname = $1 OR email = $2";
    else
        pgGetUserIdSQL = "SELECT user_id FROM auth.users WHERE nickname = $1";

	PGConnection*	pg = GDBConnectionPool->PopPG();

	pg->AddValue(nickname);
    if (!skip_email)
	    pg->AddValue(email);
    
	if (pg->ExecuteSQL(pgGetUserIdSQL) == false)
	{
		cout << "Fail To ExecuteSQL in Handle_C_SIGNUP" << endl;
		GDBConnectionPool->Push(&pg);
        session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	int	rowCount = pg->GetRowCount();
	pg->Clear();

	GDBConnectionPool->Push(&pg);

	if (rowCount == 0)// ID,email 중복 없음 생성가능
	{
		string	hashedPassword = BCrypt::generateHash(pkt.password(), 10);
        if (skip_email) {//이메일 스킵했으면 바로 pg에 계정 등록시도하고 response 전달
            if (true == SignupToPG(nickname, hashedPassword, email, skip_email))
                response.set_success(true);
            session->Send(ClientPacketHandler::MakeWriteBuffer(response));
            return;
        }
		string	temp_id = Utils::GetRandomStr(32);
	
		string	redisSetNickname = "SET %s:nickname %s EX 300";
		string	redisSetPassword = "SET %s:password %s EX 300";
		string	redisSetEmail = "SET %s:email %s EX 300";

		RedisConnection* redis = GDBConnectionPool->PopRedis();

		if (false == redis->Execute(redisSetNickname, temp_id.c_str(), nickname.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
            session->Send(ClientPacketHandler::MakeWriteBuffer(response));
			return;
		}
		redis->Clear();

		if (false == redis->Execute(redisSetPassword, temp_id.c_str(), hashedPassword.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
            session->Send(ClientPacketHandler::MakeWriteBuffer(response));
			return;
		}
		redis->Clear();

		if (false == redis->Execute(redisSetEmail, temp_id.c_str(), email.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
            session->Send(ClientPacketHandler::MakeWriteBuffer(response));
			return;
		}
		redis->Clear();
		GDBConnectionPool->Push(&redis);

		response.set_success(true);
		response.set_temp_id(temp_id);
	}
    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::C_VERIFY_MAIL_REQ& pkt)
{
	cout << "Handle_C_VERIFY_MAIL_REQ" << endl;
	Protocol::S_VERIFY_MAIL_REQ	response;
    response.set_success(false);

	string	temp_id = pkt.temp_id();//TODO 임시id 유효성 검사
	string	verfiy_code = Utils::GetRandomStr(8);// "12341234" 더미클라이언트 테스트용 고정코드

	string	redisGetEmail = "GET %s:email";
	string	redisSetVerifyCode = "SET %s:verify_code %s EX 180";

	RedisConnection* redis = GDBConnectionPool->PopRedis();

	if (false == redis->Execute(redisGetEmail, temp_id.c_str()) || redis->IsNull())
	{
		redis->Clear();
        if (redis->IsNull())
		    response.set_expired(true);
		GDBConnectionPool->Push(&redis);
	    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a1_email = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisSetVerifyCode, temp_id.c_str(), verfiy_code.c_str()))
	{
		redis->Clear();
		GDBConnectionPool->Push(&redis);
	    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a2_status = redis->GetStr();
	redis->Clear();

	GDBConnectionPool->Push(&redis);
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
	response.set_success(false);
    response.set_expired(false);

	string	temp_id = pkt.temp_id();//TODO 임시id 유효성 검사
	string	verify_code = pkt.verify_code();
	
	string	redisGetVerifyCode = "GET %s:verify_code";
	string	redisGetNickname = "GET %s:nickname";
	string	redisGetPassword = "GET %s:password";
	string	redisGetEmail = "GET %s:email";

	RedisConnection* redis = GDBConnectionPool->PopRedis();

	if (false == redis->Execute(redisGetVerifyCode, temp_id.c_str()) || redis->IsNull())
	{//레디스 실패했거나 만료됐다면
		redis->Clear();
        if (redis->IsNull())// 만료된거면
		    response.set_expired(true);
		GDBConnectionPool->Push(&redis);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a1_verify_code = redis->GetStr();
	redis->Clear();

	if (verify_code != a1_verify_code)
	{//인증코드가 틀렸을 경우
		GDBConnectionPool->Push(&redis);
		response.set_expired(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	/* 인증코드가 맞았을 경우 */
	if (false == redis->Execute(redisGetNickname, temp_id.c_str()) || redis->IsNull())
	{
		redis->Clear();
        if (redis->IsNull())// 만료된거면
		    response.set_expired(true);
		GDBConnectionPool->Push(&redis);
	    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a2_nickname = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisGetPassword, temp_id.c_str()) || redis->IsNull())
	{
		redis->Clear();
        if (redis->IsNull())// 만료된거면
		    response.set_expired(true);
		GDBConnectionPool->Push(&redis);
	    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a3_password = redis->GetStr();
	redis->Clear();

	if (false == redis->Execute(redisGetEmail, temp_id.c_str()) || redis->IsNull())
	{
		redis->Clear();
        if (redis->IsNull())// 만료된거면
		    response.set_expired(true);
		GDBConnectionPool->Push(&redis);
	    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}
	string	a4_email = redis->GetStr();
	redis->Clear();
	
	GDBConnectionPool->Push(&redis);

    if (SignupToPG(a2_nickname, a3_password, a4_email, false)) {
	    response.set_success(true);
        response.set_nickname(a2_nickname);//더미클라이언트 닉네임은 안보내도 되는데 테스트용
    }
	session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}

void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	cout << "Handle_C_LOGIN" << endl;
	Protocol::S_LOGIN	response;
    response.set_success(false);
    response.set_is_block(false);

	string			nickname = pkt.nickname();
	string			password = pkt.password();

	string		pgGetUserData = "SELECT user_id, password, is_block FROM auth.users WHERE nickname = $1";
	PGConnection*	pg = GDBConnectionPool->PopPG();
	
	pg->AddValue(nickname);

	if(pg->ExecuteSQL(pgGetUserData) == false)
	{
		cout << "Fail To ExecuteSQL in Handle_C_LOGIN" << endl;
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	if (pg->GetRowCount() == 0)
	{//존재하지 않는 nickname
		pg->Clear();
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		return;
	}

	string	a_user_id = pg->GetValue(0, 0);
	string	a_password = pg->GetValue(0, 1);
	string	a_is_block = pg->GetValue(0, 2);
	pg->Clear();
	
	GDBConnectionPool->Push(&pg);

    if (a_is_block == "t")
    {//블록된 유저라면
        response.set_is_block(true);
        session->Send(ClientPacketHandler::MakeWriteBuffer(response));
        return;
    }

	if (BCrypt::validatePassword(password, a_password))
	{//로그인 성공
		string	token = Utils::CreateAccessToken(a_user_id, nickname);
		response.set_success(true);
		response.set_token(token);
	}
	else
	{//비밀번호 틀림 레디스에 실패횟수 기록
		string	redisIncrFailCount = "INCR %s:fail_count EX 600";
		RedisConnection*	redis = GDBConnectionPool->PopRedis();
		if(false == redis->Execute(redisIncrFailCount, nickname.c_str()))
		{
			redis->Clear();
			GDBConnectionPool->Push(&redis);
            session->Send(ClientPacketHandler::MakeWriteBuffer(response));
			return;
		}
		int	fail_count = redis->GetInt();
        response.set_fail_count(fail_count);
		redis->Clear();
		if (fail_count >= 5)
		{
			string pgBlockSql = "UPDATE auth.users SET is_block = true WHERE nickname = $1";
	        PGConnection*	pg = GDBConnectionPool->PopPG();
            pg->AddValue(nickname);
            pg->ExecuteSQL(pgBlockSql);
		}
	}
    session->Send(ClientPacketHandler::MakeWriteBuffer(response));
}