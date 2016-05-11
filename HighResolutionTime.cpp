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
    Microseconds GetTime()
    {
        static QueryPerformanceInfo info = GetQueryPerformanceInfo();
        if (info.CanUseQueryPerformanceCounter)
        {
            LARGE_INTEGER counts;
            if (QueryPerformanceCounter(&counts))
            {
				return counts.QuadPart * 1000000 / info.CountsPerSecond;
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
