#pragma once

#include <iostream>
#include <cstdint>
#include <cassert>
#include <iomanip>
#include <vector>


namespace Profiller {
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

#if _WIN32
    // NOTE: This includes are hacky cause this fixes the problem of name clashing of windows API and Raylib
    // probably should be handled and moved out of namespace
    #include <intrin.h>
    #include <windows.h>
    #include <psapi.h>

    u64 GetOSTimerFreq(void)
    {
        LARGE_INTEGER Freq;
        QueryPerformanceFrequency(&Freq);
        return Freq.QuadPart;
    }

    u64 ReadOSTimer(void)
    {
        LARGE_INTEGER Value;
        QueryPerformanceCounter(&Value);
        return Value.QuadPart;
    }
#else // _WIN32
    #include <x86intrin.h>
    #include <sys/time.h>
    
    u64 GetOSTimerFreq(void)
    {
        return 1000000;
    }

    u64 ReadOSTimer(void)
    {
        timeval Value;
        gettimeofday(&Value, 0);
        
        u64 Result{ GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec };
        return Result;
    }
#endif // _WIN32

u64 ReadCPUTimer(void)
{
    return __rdtsc();
}

u64 EstimateCPUTimerFreq(void)
{
    u64 MillisecondsToWait = 100;
    u64 OSFreq = GetOSTimerFreq();

    u64 CPUStart = ReadCPUTimer();
    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while(OSElapsed < OSWaitTime)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }
    
    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;
    
    u64 CPUFreq = 0;
    if(OSElapsed)
    {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }
    
    return CPUFreq;
}



#ifndef PROFILLER
#define PROFILLER 1
#endif

#if PROFILLER

struct ProfillingAnchor
{
    u64 elapsedExclusiveTSC; // Does NOT include children
    u64 elapsedInclusiveTSC; // DOES include children
    u64 processedBytesCount;
    u64 hitCount;
    const char* label;
};
constexpr auto b{ sizeof(ProfillingAnchor)};
constexpr auto a{ sizeof(ProfillingAnchor) * (4096) / 1024.0f * (2000) };
ProfillingAnchor globalProfillingAnchors[4096];
u32 globalProfillerAnchorParent;

u32 nextEmptyAnchorIndex{ 1 };
int GetNextAcnhorIndex()
{
    return nextEmptyAnchorIndex++;
}

struct ProfillingBlock
{
    ProfillingBlock(const char* _label, u32 _byteCount, u32 _anchorIndex)
        : mLabel{ _label },
          mAnchorIndex{ _anchorIndex },
          mAnchorParentIndex{ globalProfillerAnchorParent },
          mStartTSC{ ReadCPUTimer() }
    {
        // This profilling block will be a parent for the next block
        globalProfillerAnchorParent = mAnchorIndex;

        auto* anchor{ &globalProfillingAnchors[mAnchorIndex] };
        mOldElapsedInclusiveTSC = anchor->elapsedExclusiveTSC;
        anchor->processedBytesCount += _byteCount;
    }

    ~ProfillingBlock()
    {
        u64 elapsedTSC{ ReadCPUTimer() - mStartTSC };

        auto* parentAnchor{ &globalProfillingAnchors[mAnchorParentIndex] };
        parentAnchor->elapsedExclusiveTSC -= elapsedTSC;
        globalProfillerAnchorParent = mAnchorParentIndex;

        auto* anchor{ &globalProfillingAnchors[mAnchorIndex] };
        anchor->elapsedExclusiveTSC += elapsedTSC;
        anchor->elapsedInclusiveTSC = elapsedTSC + mOldElapsedInclusiveTSC;
        anchor->hitCount++;
        anchor->label = mLabel;
    }

    const char* mLabel;
    u32 mAnchorIndex;
    u32 mAnchorParentIndex;
    
    u64 mStartTSC;
    u64 mOldElapsedInclusiveTSC;
};

struct AnchorTimingsResult
{
    ProfillingAnchor mAnchor;
    u64 mElapsedTSC{ 0 };
    f64 mElapsedTimeInS{ 0.0f };
    f64 mPercentage{ 0.0f };
    f64 mPercentageWithChildren{ 0.0f };
    f64 mProcessedMegabytes;
    f64 mGigabytesPerSecond;
};

inline u32 GetLastAnchorIndex();
#define ANCHOR_LAST_INDEX_DEFINITION inline u32 Profiller::GetLastAnchorIndex() { return __COUNTER__; }

std::vector<AnchorTimingsResult> GetAnchorsTimings(u64 _totalCyclesPassed)
{
    std::vector<AnchorTimingsResult> results;
    u32 resultsSize{ GetLastAnchorIndex() };
    results.reserve(resultsSize);

    f64 cpuFreq{ (f64)EstimateCPUTimerFreq() };
    for (size_t i = 0; i < resultsSize; i++)
    {
        const auto& a{ globalProfillingAnchors[i] };
        if (a.hitCount > 0)
        {
            u64 elapsedTSC{ a.elapsedExclusiveTSC };
            f64 elapsedTimeInS{ elapsedTSC / cpuFreq };
            f64 percentage{ (f64)elapsedTSC / (f64)_totalCyclesPassed * 100.0 };
            //std::cout << _outputPrefix << ' ' << std::to_string(i) << ' '
            //    << a.label 
            //    << " [" << a.hitCount << "]: "
            //    << elapsedTSC << ' '
            //    << elapsedTimeInS << 's'
            //    << " (" << percentage << '%';

            f64 percentageWithChildren;
            if (a.elapsedInclusiveTSC != a.elapsedExclusiveTSC)
            {
                percentageWithChildren = (f64)a.elapsedInclusiveTSC / (f64)_totalCyclesPassed * 100.0;
                // std::cout << ", " << percentageWithChildren << "% w/children";
            }
            // std::cout << ") ";

            f64 processedMegabytes;
            f64 gigabytesPerSecond;
            if (a.processedBytesCount > 0)
            {
                constexpr f64 megabyte{ 1024.0f * 1024.0f };
                constexpr f64 gigabyte{ megabyte * 1024.0f };

                f64 seconds{ (f64)a.elapsedInclusiveTSC / cpuFreq };
                f64 bytesPerSecond{ (f64)a.processedBytesCount / seconds };
                processedMegabytes = (f64)a.processedBytesCount / megabyte;
                gigabytesPerSecond = bytesPerSecond / gigabyte;
                // std::cout << processedMegabytes << " mbs, at " << gigabytesPerSecond << " gb/s";
            }
            // std::cout << '\n';
            results.emplace_back(
                a,
                elapsedTSC,
                elapsedTimeInS,
                percentage,
                percentageWithChildren,
                processedMegabytes,
                gigabytesPerSecond
            );
        }
    }
    return results;
}

void PrintAnchorsTimings(const char* _outputPrefix, u64 _totalCyclesPassed)
{
    f64 cpuFreq{ (f64)EstimateCPUTimerFreq() };
    for (size_t i = 0; i < 4096; i++)
    {
        const auto& a{ globalProfillingAnchors[i] };
        if (a.hitCount > 0)
        {
            u64 elapsedTSC{ a.elapsedExclusiveTSC };
            f64 elapsedTimeInS{ elapsedTSC / cpuFreq };
            f64 percentage{ (f64)elapsedTSC / (f64)_totalCyclesPassed * 100.0 };
            std::cout << _outputPrefix << ' ' << std::to_string(i) << ' '
                << a.label 
                << " [" << a.hitCount << "]: "
                << elapsedTSC << ' '
                << elapsedTimeInS << 's'
                << " (" << percentage << '%';

            if (a.elapsedInclusiveTSC != a.elapsedExclusiveTSC)
            {
                f64 percentageWithChildren{ (f64)a.elapsedInclusiveTSC / (f64)_totalCyclesPassed * 100.0 };
                std::cout << ", " << percentageWithChildren << "% w/children";
            }
            std::cout << ") ";

            if (a.processedBytesCount > 0)
            {
                constexpr f64 megabyte{ 1024.0f * 1024.0f };
                constexpr f64 gigabyte{ megabyte * 1024.0f };

                f64 seconds{ (f64)a.elapsedInclusiveTSC / cpuFreq };
                f64 bytesPerSecond{ (f64)a.processedBytesCount / seconds };
                f64 processedMegabytes{ (f64)a.processedBytesCount / megabyte };
                f64 gigabytesPerSecond{ bytesPerSecond / gigabyte };
                std::cout << processedMegabytes << " mbs, at " << gigabytesPerSecond << " gb/s";
            }
            std::cout << '\n';
        }
    }
}

#define CONCAT(str1, str2)                  str1 ## str2
#define CONCAT2(str1, str2)                 CONCAT(str1, str2)
#define ANCHOR_INDEX_VAR                    CONCAT2(anchorIndex, __LINE__)
// ===== Crude implementation for projects with multiple compilation units
//#define TIME_BANDWIDTH(blockName, byteCount) \
//    static int ANCHOR_INDEX_VAR = Profiller::GetNextAcnhorIndex(); \
//    const Profiller::ProfillingBlock CONCAT2(timeFunction, __LINE__)(blockName, byteCount, ANCHOR_INDEX_VAR)
// ===== Implementation for projects with single compilation unit
#define TIME_BANDWIDTH(blockName, byteCount) \
    const Profiller::ProfillingBlock CONCAT2(timeFunction, __LINE__)(blockName, byteCount, __COUNTER__ + 1)

#else // PROFILLER

#define CONCAT(...)
#define CONCAT2(...)
#define ANCHOR_INDEX_VAR
#define TIME_BANDWIDTH(...)
#define GetLastAnchorIndex(...)
#define ANCHOR_LAST_INDEX_DEFINITION
#define PrintAnchorsTimings(...)

#endif // PROFILLER

#define TIME_FUNCTION TIME_BANDWIDTH(__FUNCTION__, 0)

class Profiller
{
public:
    void BeginProfilling(const char* _outputPrefix)
    {
        mOutputPrefix = _outputPrefix;
        mCpuFreq = EstimateCPUTimerFreq();
        mStartTSC = ReadCPUTimer();
    }

    u64 EndProfilling()
    {
        mEndTSC = ReadCPUTimer();

        u64 totalCyclesPassed{ mEndTSC - mStartTSC };
        f64 totalMillisecondsPassed{ (f64)totalCyclesPassed / (f64)mCpuFreq * 1000.0 };

        std::cout << std::fixed << std::setprecision(2);
        std::cout << mOutputPrefix << " Total time: " << totalMillisecondsPassed << "ms (CPU freq " << mCpuFreq << ")\n";

        PrintAnchorsTimings(mOutputPrefix, totalCyclesPassed);
        return totalCyclesPassed;
    }

public:
    const char* mOutputPrefix;
    u64 mStartTSC;
    u64 mEndTSC;
    u64 mCpuFreq;
};

Profiller globalProfiller;

}
