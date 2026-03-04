#include "ReadBuffer.h"
#include <cstring>

ReadBuffer::ReadBuffer(int bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

ReadBuffer::~ReadBuffer()
{

}

void	ReadBuffer::Clean()
{
	int	dataSize = DataSize();
	if (dataSize == 0)
	{
		_readPos = _writePos = 0;
	}
	else
	{
		if (FreeSize() < _bufferSize)
		{
			memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool	ReadBuffer::OnRead(int numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;
	_readPos += numOfBytes;
	return true;
}

bool	ReadBuffer::OnWrite(int numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;
	_writePos += numOfBytes;
	return true;
}