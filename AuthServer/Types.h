#pragma once

#include <cstdint>
#include <memory>

using BYTE  = unsigned char;

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>;

USING_SHARED_PTR(Service);
USING_SHARED_PTR(EpollCore);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(EpollObject);
USING_SHARED_PTR(EpollEvent);
USING_SHARED_PTR(Session);
USING_SHARED_PTR(Timer);
USING_SHARED_PTR(SslCtx);
USING_SHARED_PTR(SslObject);
USING_SHARED_PTR(Job);
USING_SHARED_PTR(JobQueue);
USING_SHARED_PTR(WriteBuffer);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(DBConnection);
