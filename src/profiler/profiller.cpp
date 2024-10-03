#include "profiller.hpp"

#include <cstdint>
#include <limits>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

constexpr u8  u8_MAX { std::numeric_limits<uint8_t>::max()  };
constexpr u16 u16_MAX{ std::numeric_limits<uint16_t>::max() };
constexpr u32 u32_MAX{ std::numeric_limits<uint32_t>::max() };
constexpr u64 u64_MAX{ std::numeric_limits<uint64_t>::max() };

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

#if _WIN32
    #include <intrin.h>
    #include <windows.h>
    #include <psapi.h>

    u64 Profiller::GetOSTimerFreq(void)
    {
        LARGE_INTEGER Freq;
        QueryPerformanceFrequency(&Freq);
        return Freq.QuadPart;
    }

    u64 Profiller::ReadOSTimer(void)
    {
        LARGE_INTEGER Value;
        QueryPerformanceCounter(&Value);
        return Value.QuadPart;
    }
#else // _WIN32
    #include <x86intrin.h>
    #include <sys/time.h>
    
    u64 Profiller::GetOSTimerFreq(void)
    {
        return 1000000;
    }

    u64 Profiller::ReadOSTimer(void)
    {
        timeval Value;
        gettimeofday(&Value, 0);
        
        u64 Result{ GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec };
        return Result;
    }
#endif // _WIN32

u64 Profiller::ReadCPUTimer(void)
{
    return __rdtsc();
}
