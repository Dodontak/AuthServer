#pragma once

#include "Types.h"
#include <vector>

class WriteBuffer
{
public:
	WriteBuffer(BYTE* buffer, uint16 dataLen);
	WriteBuffer(uint16 dataLen);//for AppendBuffer
	~WriteBuffer();

public: //use at fill buffer
	bool	AppendBuffer(BYTE* buffer, uint16 dataLen);
	BYTE*	GetCopyBuffer() { return &_buffer[_copyPos]; }

public: //use at write
	BYTE*	GetBuffer() { return &_buffer[_writePos]; }
	int		GetDataLen() { return _allocSize - _writePos; }
	bool	UpdateWritePos(int writeLen);
	
private:
	int					_writePos;
	int					_copyPos;
	int					_allocSize;
	std::vector<BYTE>	_buffer;
};