#pragma once

#include "Types.h"
#include <vector>

class WriteBuffer
{
public:
	WriteBuffer(BYTE* buffer, int dataLen);
	~WriteBuffer();

	BYTE*	GetBuffer() { return &_buffer[_writePos]; }
	int		GetDataLen() { return _allocSize - _writePos; }
	bool	UpdateWritePos(int writeLen);
	
private:
	int					_writePos;
	int					_allocSize;
	std::vector<BYTE>	_buffer;
};