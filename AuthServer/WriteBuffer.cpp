#include "WriteBuffer.h"
#include <cstring>
#include <iostream>

using namespace std;

WriteBuffer::WriteBuffer(BYTE* buffer, int dataLen) : _writePos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
	memcpy(&_buffer[0], buffer, dataLen);
}

WriteBuffer::~WriteBuffer()
{
}

bool	WriteBuffer::UpdateWritePos(int writeLen)
{
	_writePos += writeLen;
	if (_allocSize <= _writePos)
		return false;
	return true;
}
