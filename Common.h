#pragma once
#ifndef COMMON_H
#define COMMON_H

// For mingw32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#define WM_INPUT_DEVICE_CHANGE 0x00FE
#include "mingw-std-threads/mingw.mutex.h"
#include "mingw-std-threads/mingw.thread.h"

// C++ libs
#include <queue>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <map>
#include <atomic>

// C libs
#include <stdio.h>
#include <windows.h>

template<typename T> T min(T x, T y)
{
    return x < y ? x : y;
}
template<typename T> T max(T x, T y)
{
    return x > y ? x : y;
}

using std::int32_t;
using std::int16_t;

#endif
