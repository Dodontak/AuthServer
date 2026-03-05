#include "ServerPacketHandler.h"
#include <iostream>

std::function<bool(std::function<void()>&, PacketSessionRef, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_SIGNUP(PacketSessionRef session, Protocol::C_SIGNUP pkt)
{

}

void	Handle_S_VERIFY_EMAIL(PacketSessionRef session, Protocol::C_VERIFY_EMAIL pkt)
{

}

void	Handle_S_LOGIN(PacketSessionRef session, Protocol::C_LOGIN pkt)
{

}