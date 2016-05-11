#pragma once
#ifndef INPUT_DEVICE_LIST_H
#define INPUT_DEVICE_LIST_H

#include "Common.h"
#include "InputDevice.h"

class InputDeviceList
{
    friend class InputDevice;
private:
    WNDPROC baseWndProc;
    int HandleRawInput(LPARAM lparam);
public:
    std::vector<InputDevice *> devices;
    InputDeviceList();
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void InterceptEvents(HWND windowHandle);
};

#endif
