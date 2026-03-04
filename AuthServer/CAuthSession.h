#pragma once

#include "Session.h"

class CAuthSession : public PacketSession
{
public:
private:
	virtual void	OnWrite(int len) override;
	virtual void	OnReadPacket(BYTE* buffer, int len) override;
};