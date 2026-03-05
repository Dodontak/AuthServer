#pragma once

#include "Types.h"
#include <vector>

class ReadBuffer
{
	enum { BUFFER_COUNT = 10 };
public:
	ReadBuffer(int bufferSize);
	~ReadBuffer();

	bool				OnRead(int numOfBytes);
	bool				OnWrite(int numOfBytes);

	void				Clean();
	BYTE*				ReadPos() { return &_buffer[_readPos]; }
	BYTE*				WritePos() { return &_buffer[_writePos]; }
	int					DataSize() { return _writePos - _readPos; }
	int					FreeSize() { return _capacity - _writePos; }
private:
	int					_capacity = 0;
	int					_bufferSize = 0;
	int					_readPos = 0;
	int					_writePos = 0;
	std::vector<BYTE>	_buffer;
};
