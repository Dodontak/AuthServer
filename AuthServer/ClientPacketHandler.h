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
	PKT_C_SIGNUP = 1000,
	PKT_S_SIGNUP = 1001,
	PKT_C_VERIFY_MAIL_REQ = 1002,
	PKT_S_VERIFY_MAIL_REQ = 1003,
	PKT_C_VERIFY_EMAIL_CODE = 1004,
	PKT_S_VERIFY_EMAIL_CODE = 1005,
	PKT_C_LOGIN = 1006,
	PKT_S_LOGIN = 1007,
};

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len);
void	Handle_C_SIGNUP(const PacketSessionRef& session, const Protocol::C_SIGNUP& pkt);
void	Handle_C_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::C_VERIFY_MAIL_REQ& pkt);
void	Handle_C_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::C_VERIFY_EMAIL_CODE& pkt);
void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt);

class ClientPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_SIGNUP] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_SIGNUP>(outFunc, Handle_C_SIGNUP, session, buffer, len);
		};
		GPacketHandler[PKT_C_VERIFY_MAIL_REQ] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_VERIFY_MAIL_REQ>(outFunc, Handle_C_VERIFY_MAIL_REQ, session, buffer, len);
		};
		GPacketHandler[PKT_C_VERIFY_EMAIL_CODE] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_VERIFY_EMAIL_CODE>(outFunc, Handle_C_VERIFY_EMAIL_CODE, session, buffer, len);
		};
		GPacketHandler[PKT_C_LOGIN] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_LOGIN>(outFunc, Handle_C_LOGIN, session, buffer, len);
		};
	}

	static bool	PacketHandler(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static WriteBufferRef MakeWriteBuffer(Protocol::S_SIGNUP& pkt) { return MakeWriteBuffer(pkt, PKT_S_SIGNUP); }
	static WriteBufferRef MakeWriteBuffer(Protocol::S_VERIFY_MAIL_REQ& pkt) { return MakeWriteBuffer(pkt, PKT_S_VERIFY_MAIL_REQ); }
	static WriteBufferRef MakeWriteBuffer(Protocol::S_VERIFY_EMAIL_CODE& pkt) { return MakeWriteBuffer(pkt, PKT_S_VERIFY_EMAIL_CODE); }
	static WriteBufferRef MakeWriteBuffer(Protocol::S_LOGIN& pkt) { return MakeWriteBuffer(pkt, PKT_S_LOGIN); }

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