#pragma once
#ifndef INPUT_H
#define INPUT_H

#include "Common.h"

struct DeviceInfo
{
    int16_t DeviceId;

	int32_t UsbVendorId;
	int32_t UsbProductId;

    const char *Description;
    const char *Driver;
};

namespace Input
{
    void StartListening(HWND window);
}

#endif
