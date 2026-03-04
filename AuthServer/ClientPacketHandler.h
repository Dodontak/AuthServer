#pragma once

#include "Types.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "WriteBuffer.h"

class ClientPacketHandler
{
public:
	static Protocol::C_SignUp		HandeSignUpPacket(BYTE* buffer, int len);
	static Protocol::C_VerifyEmail	HandeVerifyEmailPacket(BYTE* buffer, int len);
	static Protocol::C_Login		HandeLoginPacket(BYTE* buffer, int len);

	// template<typename T>
	// static WriteBufferRef	MakeWriteBuffer(T& pkt)
	// {
	// }
};