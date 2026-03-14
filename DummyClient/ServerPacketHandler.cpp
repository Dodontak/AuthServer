#include "ServerPacketHandler.h"
#include <thread>
#include <jwt-cpp/jwt.h>

using namespace std;

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool VerifyAccessToken(const string& token, string& out_user_id)
{
    const string SECRET_KEY = getenv("JWT_SECRET_KEY");
    
    try
    {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .with_issuer("auth_server");    // 발급자 확인
        
        auto decoded = jwt::decode(token);
        verifier.verify(decoded);           // 서명 + 만료시간 자동 검증
        
        out_user_id = decoded.get_payload_claim("user_id").as_string();
        return true;
    }
    catch (const exception& e)
    {
		cout << "exeption" << endl;
        // 서명 불일치, 만료, 형식 오류 전부 여기로 떨어짐
        return false;
    }
}

bool Handle_INVALID(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_SIGNUP(const PacketSessionRef& session, const Protocol::S_SIGNUP& pkt)
{
	Protocol::C_VERIFY_MAIL_REQ	response;
	bool	success = pkt.success();
	string	temp_id = pkt.temp_id();
	if (success)
	{
		response.set_temp_id(temp_id);
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//실제론 disconnect할 필요는 없을듯
		session->Disconnect();
		return;
	}
}

void	Handle_S_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::S_VERIFY_MAIL_REQ& pkt)
{
	Protocol::C_VERIFY_EMAIL_CODE	response;
	bool	success = pkt.success();
	string	temp_id = pkt.temp_id();
	if (success)
	{
		response.set_temp_id(temp_id);
		response.set_verify_code("12341234");
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//실제론 disconnect할 필요는 없을듯
		session->Disconnect();
		return;
	}
}

void	Handle_S_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::S_VERIFY_EMAIL_CODE& pkt)
{
	Protocol::C_LOGIN	response;
	bool	success = pkt.success();
	bool	expired = pkt.expired();
	string	nickname = pkt.nickname();

	if (success)
	{
		response.set_nickname(nickname);
		response.set_password("asdqwezxc123");
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		if (expired)
		{
			cout << "verify code expired!" << endl;
		}
		else
		{
			cout << "verify code wrong!" << endl;
		}
		//실제론 disconnect할 필요는 없을듯
		session->Disconnect();
	}
}

void	Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt)
{
	bool	success = pkt.success();
	if (success)
	{
		string	token = pkt.token();
		string	user_id;
		if (token == "token"/*VerifyAccessToken(token, user_id)*/)//토큰부분 잠시 제거
		{
			cout << "login suceess! user : " << user_id << endl;
		}
	}
	else
	{
		cout << "login fail!" << endl;
		session->Disconnect();
	}
}