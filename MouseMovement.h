#pragma once
#ifndef MOUSE_MOVEMENT_H
#define MOUSE_MOVEMENT_H

#include "Common.h"
#include "HighResolutionTime.h"

struct MouseMovement
{
    int DX;
    int DY;
    HighResolutionTime::Microseconds Microseconds;
};

#endif
