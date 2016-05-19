#pragma once
#ifndef EVENTS_H
#define EVENTS_H

#include "Common.h"
#include "HighResolutionTime.h"

struct Event
{
    int16_t DeviceId;
    Microseconds Time;
    int16_t DX;
    int16_t DY;
};

namespace Events
{
    const std::vector<Event> &SwapBuffer();
    void Buffer(Event event);
}

#endif
