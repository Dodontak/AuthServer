#include "WriteBuffer.h"
#include <cstring>
#include <iostream>

using namespace std;

WriteBuffer::WriteBuffer(BYTE* buffer, uint16 dataLen) : _writePos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
	memcpy(&_buffer[0], buffer, dataLen);
}

WriteBuffer::WriteBuffer(uint16 dataLen) : _writePos(0),  _copyPos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
}

WriteBuffer::~WriteBuffer() {}

bool	WriteBuffer::AppendBuffer(BYTE* buffer, uint16 dataLen)
{
	memcpy(&_buffer[_copyPos], buffer, dataLen);
	_copyPos += dataLen;
	return true;
}


bool	WriteBuffer::UpdateWritePos(int writeLen)
{
	_writePos += writeLen;
	if (_allocSize <= _writePos)
		return false;
	return true;
}
