#include "ClientPacketHandler.h"
#include "Types.h"
#include "Timer.h"
#include <thread>
#include <string>

std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt)
{
	std::string	email, nickname, password;
	email = pkt.email();
	nickname = pkt.nickname();
	password = pkt.password();
	/* TODO DB에서 email, nickname 이미 있는지 확인.
	중복 있으면 실패, disconnect(해야할까?). 없으면 다음 단계로.
	*/
	std::this_thread::sleep_for(std::chrono::milliseconds(100));//DB작업(0.1초 걸렸다 침)

	Protocol::S_SIGNUP	response;
	if (true)
	{
		response.set_success(true);
		//중복 없으면 성공했다고 알림. 이메일인증쪽은 고민좀 해보자.
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		response.set_success(false);
		//중복있으면 실패했다고 알림
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
}

void	Handle_C_VERIFY_EMAIL(const PacketSessionRef& session, const Protocol::C_VERIFY_EMAIL& pkt)
{
	std::string	email = pkt.email();
	std::string	code = pkt.verification_code();
	// 서버에서 준 코드와 클라이언트가 보낸 코드가 일치하는지 확인
	std::this_thread::sleep_for(std::chrono::milliseconds(100));//DB작업(0.1초 걸렸다 침)
	
	Protocol::S_VERIFY_EMAIL	response;

	if (code == code)
	{
		//코드 일치하면 postgres에 새 계정 정보 INSERT 시도.
		//INSERT 성공 시 성공 패킷 보냄. 실패 시 실패 패킷 보냄.
		response.set_success(true);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		//코드 일치하지 않으면 실패 패킷 보냄
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		//disconnect?
	}
}

void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	std::string	nickname = pkt.nickname();
	std::string	password = pkt.password();
	//password 해싱해서 postgres에 있는 해싱된 password와 일치하는지 확인.
	//일치하면 클라이언트에게 로그인토큰 발급해줌, redis에 로그인토큰 저장
	std::this_thread::sleep_for(std::chrono::milliseconds(100));//DB작업(0.1초 걸렸다 침)
	Protocol::S_LOGIN	response;
	if (true)
	{
		response.set_success(true);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeWriteBuffer(response));
		session->Disconnect();
	}
}