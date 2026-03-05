#include "ClientPacketHandler.h"
#include <iostream>

std::function<bool(std::function<void()>&, PacketSessionRef, BYTE*, int32)> GPacketHandler[UINT16_MAX];


bool Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
{
	return false;
}

void Handle_C_SIGNUP(PacketSessionRef session, Protocol::C_SIGNUP pkt)
{
	std::cout << "Handle_C_SIGNUP!" << std::endl;
}

void Handle_C_VERIFY_EMAIL(PacketSessionRef session, Protocol::C_VERIFY_EMAIL pkt)
{
	std::cout << "Handle_C_VERIFY_EMAIL!" << std::endl;
}

void Handle_C_LOGIN(PacketSessionRef session, Protocol::C_LOGIN pkt)
{
	std::cout << "Handle_C_LOGIN!" << std::endl;
}