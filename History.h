#pragma once
#ifndef HISTORY_H
#define HISTORY_H
#include "Common.h"

namespace History
{
    extern std::vector< std::vector<MouseMove> > moves;
    extern void acquirelock();
    extern void releaselock();
    extern int done;
    extern int adjustsens;
}

#endif
