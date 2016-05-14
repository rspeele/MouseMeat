#pragma once
#ifndef OUTPUT_H
#define OUTPUT_H

#include "Common.h"
#include "Events.h"
#include "Input.h"

namespace Output
{
    void OutputDeviceInfo(const DeviceInfo &device);
    void OutputEvent(const Event &event);
}

#endif
