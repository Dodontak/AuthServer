#include "ServerPacketHandler.h"
#include <thread>

using namespace std;

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_AS_SIGNUP(const PacketSessionRef& session, const Protocol::AS_SIGNUP& pkt)
{
    cout << "Handle_AS_SIGNUP" << endl;
	Protocol::AC_VERIFY_MAIL_REQ	response;
	bool	success = pkt.success();
	string	temp_id = pkt.temp_id();
    bool    skip_email = pkt.skip_email();

	if (success)
	{
        if (skip_email)
        {//이메일 스킵이면 계정 등록 성공이니 disconnect
            session->Disconnect();
            return;
        }
		response.set_temp_id(temp_id);
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//실제론 disconnect할 필요는 없을듯
		session->Disconnect();
		return;
	}
}

void	Handle_AS_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::AS_VERIFY_MAIL_REQ& pkt)
{
    cout << "Handle_AS_VERIFY_MAIL_REQ" << endl;
	Protocol::AC_VERIFY_EMAIL_CODE	response;
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

void	Handle_AS_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::AS_VERIFY_EMAIL_CODE& pkt)
{
    cout << "Handle_AS_VERIFY_EMAIL_CODE" << endl;
	Protocol::AC_LOGIN	response;
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

void	Handle_AS_LOGIN(const PacketSessionRef& session, const Protocol::AS_LOGIN& pkt)
{
    cout << "Handle_AS_LOGIN" << endl;
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