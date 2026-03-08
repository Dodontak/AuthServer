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
{%- for pkt in parser.total_pkt %}
	PKT_{{pkt.name}} = {{pkt.id}},
{%- endfor %}
};

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len);
{%- for pkt in parser.recv_pkt %}
void	Handle_{{pkt.name}}(const PacketSessionRef& session, const Protocol::{{pkt.name}}& pkt);
{%- endfor %}

class {{output}}
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
{%- for pkt in parser.recv_pkt %}
		GPacketHandler[PKT_{{pkt.name}}] = [](std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::{{pkt.name}}>(outFunc, Handle_{{pkt.name}}, session, buffer, len);
		};
{%- endfor %}
	}

	static bool	PacketHandler(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}

{%- for pkt in parser.send_pkt %}
	static WriteBufferRef MakeWriteBuffer(Protocol::{{pkt.name}}& pkt) { return MakeWriteBuffer(pkt, PKT_{{pkt.name}}); }
{%- endfor %}

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