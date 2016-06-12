#include "HighResolutionTime.h"

namespace HighResolutionTime
{
    struct QueryPerformanceInfo
    {
        bool CanUseQueryPerformanceCounter;
        LONGLONG CountsPerSecond;
    };
    QueryPerformanceInfo GetQueryPerformanceInfo()
    {
        auto info = QueryPerformanceInfo();
        LARGE_INTEGER countsPerSecond;
        if (QueryPerformanceFrequency(&countsPerSecond))
        {
            info.CanUseQueryPerformanceCounter = true;
            info.CountsPerSecond = countsPerSecond.QuadPart;
        }
        else
        {
            info.CanUseQueryPerformanceCounter = false;
        }
        return info;
    }
    Microseconds start = 0;
    Microseconds GetTime()
    {
        static QueryPerformanceInfo info = GetQueryPerformanceInfo();
        if (info.CanUseQueryPerformanceCounter)
        {
            LARGE_INTEGER counts;
            if (QueryPerformanceCounter(&counts))
            {
				Microseconds micros = counts.QuadPart * 1000000 / info.CountsPerSecond;
                if (!start) start = micros;
                return micros - start;
            }
			else
			{
				throw std::runtime_error
                    ("QPC was supposed to work, but didn't.");
			}
        }
        else
        {
            throw std::runtime_error
                ("QPC not supported");
        }
    }
}
