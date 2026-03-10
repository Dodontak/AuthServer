#include "ServerPacketHandler.h"
#include <thread>

std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_SIGNUP(const PacketSessionRef& session, const Protocol::S_SIGNUP& pkt)
{
	std::cout << "Handle_S_SIGNUP" << std::endl;
}

void	Handle_S_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::S_VERIFY_MAIL_REQ& pkt)
{

}

void	Handle_S_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::S_VERIFY_EMAIL_CODE& pkt)
{

}

void	Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt)
{
	
}