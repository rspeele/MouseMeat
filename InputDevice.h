#pragma once
#ifndef INPUT_DEVICE_H
#define INPUT_DEVICE_H

#include "Common.h"
#include "MouseMovement.h"

class InputDeviceList;

class InputDevice
{
private:
    HANDLE handle;
    DWORD type;
    std::vector<BYTE> *deviceDesc;
	std::vector<BYTE> *driver;
    std::queue<MouseMovement> *moves;
    std::mutex *mutex;
public:
    ~InputDevice();
	int UsbVendorId;
	int UsbProductId;
	const char *DeviceDesc();
	const char *Driver();
    bool IsDevice(HANDLE handle);
    void EnqueueMouseMovement(MouseMovement movement);
    void DequeueMouseMovements(std::vector<MouseMovement> &movements);
    static InputDeviceList *GetInputDevices();
    static void RegisterKeyboard(HWND window);
    static void RegisterMouse(HWND window);
};

#endif
