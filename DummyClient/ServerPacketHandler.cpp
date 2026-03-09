#include "ServerPacketHandler.h"
#include <thread>

std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_SIGNUP(const PacketSessionRef& session, const Protocol::S_SIGNUP& pkt)
{
	bool	success = pkt.success();

	if (success == true)
	{//만들 수 있는 계정이면 이메일 인증 창 띄우기.
		Protocol::C_VERIFY_EMAIL	response;
		response.set_email("email");//이메일
		response.set_verification_code("332134");//인증코드
		std::cout << session->GetFd() << " : you can use nickname/email \n";
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));//DB작업(0.1초 걸렸다 침)
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//이미 있는 nickname / email입니다. 문구띄우기
		std::cout << session->GetFd() << " : nickname aleady used\n";
		session->Disconnect();
	}
}

void	Handle_S_VERIFY_EMAIL(const PacketSessionRef& session, const Protocol::S_VERIFY_EMAIL& pkt)
{
	bool	success = pkt.success();

	Protocol::C_LOGIN	response;
	if (success == true)
	{//이메일 인증 성공 시 
		std::cout << session->GetFd() << " : Verification Success!\n";
		response.set_nickname("Dodontak");
		response.set_password("password123");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));//DB작업(0.1초 걸렸다 침)

		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{//실패 시 재도전 가능?
		std::cout << session->GetFd() << "wrong verification code!\n";
		session->Disconnect();
	}
}

void	Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt)
{
	bool	success = pkt.success();
	if (success == true)
		std::cout << session->GetFd() << " : login Success!\n";
	else
		std::cout << session->GetFd() << " : login Failed!\n";
}