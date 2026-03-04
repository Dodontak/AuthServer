#include "ClientPacketHandler.h"

Protocol::C_SignUp	ClientPacketHandler::HandeSignUpPacket(BYTE* buffer, int len)
{
	struct PacketHeader*	header = reinterpret_cast<struct PacketHeader*>(buffer);
	Protocol::C_SignUp	pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	return pkt;
}

Protocol::C_VerifyEmail	ClientPacketHandler::HandeVerifyEmailPacket(BYTE* buffer, int len)
{
	struct PacketHeader*	header = reinterpret_cast<struct PacketHeader*>(buffer);
	Protocol::C_VerifyEmail	pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	return pkt;
}

Protocol::C_Login	ClientPacketHandler::HandeLoginPacket(BYTE* buffer, int len)
{
	struct PacketHeader*	header = reinterpret_cast<struct PacketHeader*>(buffer);
	Protocol::C_Login	pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	return pkt;
}
