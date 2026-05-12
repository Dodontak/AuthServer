#pragma once

#include "Types.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "WriteBuffer.h"
#include <memory>
#include <functional>

extern std::function<bool(std::function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_AC_SIGNUP = 20000,
	PKT_AS_SIGNUP = 20001,
	PKT_AC_VERIFY_MAIL_REQ = 20002,
	PKT_AS_VERIFY_MAIL_REQ = 20003,
	PKT_AC_VERIFY_EMAIL_CODE = 20004,
	PKT_AS_VERIFY_EMAIL_CODE = 20005,
	PKT_AC_LOGIN = 20006,
	PKT_AS_LOGIN = 20007,
};

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len);
void	Handle_AS_SIGNUP(const PacketSessionRef& session, const Protocol::AS_SIGNUP& pkt);
void	Handle_AS_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::AS_VERIFY_MAIL_REQ& pkt);
void	Handle_AS_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::AS_VERIFY_EMAIL_CODE& pkt);
void	Handle_AS_LOGIN(const PacketSessionRef& session, const Protocol::AS_LOGIN& pkt);

class ServerPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_AS_SIGNUP] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_SIGNUP>(outFunc, Handle_AS_SIGNUP, session, buffer, len);
		};
		GPacketHandler[PKT_AS_VERIFY_MAIL_REQ] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_VERIFY_MAIL_REQ>(outFunc, Handle_AS_VERIFY_MAIL_REQ, session, buffer, len);
		};
		GPacketHandler[PKT_AS_VERIFY_EMAIL_CODE] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_VERIFY_EMAIL_CODE>(outFunc, Handle_AS_VERIFY_EMAIL_CODE, session, buffer, len);
		};
		GPacketHandler[PKT_AS_LOGIN] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_LOGIN>(outFunc, Handle_AS_LOGIN, session, buffer, len);
		};
	}

	static bool	PacketHandler(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static WriteBufferRef MakeWriteBuffer(Protocol::AC_SIGNUP& pkt) { return MakeWriteBuffer(pkt, PKT_AC_SIGNUP); }
	static WriteBufferRef MakeWriteBuffer(Protocol::AC_VERIFY_MAIL_REQ& pkt) { return MakeWriteBuffer(pkt, PKT_AC_VERIFY_MAIL_REQ); }
	static WriteBufferRef MakeWriteBuffer(Protocol::AC_VERIFY_EMAIL_CODE& pkt) { return MakeWriteBuffer(pkt, PKT_AC_VERIFY_EMAIL_CODE); }
	static WriteBufferRef MakeWriteBuffer(Protocol::AC_LOGIN& pkt) { return MakeWriteBuffer(pkt, PKT_AC_LOGIN); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool	GetCallback(std::function<void()>& outFunc, ProcessFunc& func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType	pkt;
		if (false == pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)))
			return false;
		outFunc = [func, session, pkt](){ func(session, pkt); };
		return true;
	}

	template<typename T>
	static WriteBufferRef	MakeWriteBuffer(T& pkt, uint16 pktId)
	{
		int	headerSize = sizeof(PacketHeader);
		int	pktSize = pkt.ByteSizeLong();
		WriteBufferRef	writeBuffer = std::make_shared<WriteBuffer>(headerSize + pktSize);

		PacketHeader	header;
		header.id = pktId;
		header.size = headerSize + pktSize;

		writeBuffer->AppendBuffer(reinterpret_cast<BYTE*>(&header), headerSize);
		pkt.SerializeToArray(writeBuffer->GetCopyBuffer(), pktSize);
		return writeBuffer;
	}
};