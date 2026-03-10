#include "ServerPacketHandler.h"
#include <thread>

std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_SIGNUP(const PacketSessionRef& session, const Protocol::S_SIGNUP& pkt)
{
	Protocol::C_VERIFY_MAIL_REQ	response;
	bool	success = pkt.success();
	std::string	temp_id = pkt.temp_id();
	if (success)
	{
		response.set_temp_id(temp_id);
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		session->Disconnect();
		return;
	}
}

void	Handle_S_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::S_VERIFY_MAIL_REQ& pkt)
{
	Protocol::C_VERIFY_EMAIL_CODE	response;
	bool	success = pkt.success();
	std::string	temp_id = pkt.temp_id();
	if (success)
	{
		response.set_temp_id(temp_id);
		response.set_verify_code("12341234");
		session->Send(ServerPacketHandler::MakeWriteBuffer(response));
	}
	else
	{
		session->Disconnect();
		return;
	}
}

void	Handle_S_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::S_VERIFY_EMAIL_CODE& pkt)
{
	bool	success = pkt.success();
	if (success)
	{
		std::cout << "Successfully signed up!" << std::endl;
	}
	else
	{
		std::cout << "Failed to sign up!" << std::endl;
	}
	session->Disconnect();
}

void	Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt)
{
	
}