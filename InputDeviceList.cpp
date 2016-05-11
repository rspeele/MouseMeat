#include "Common.h"
#include "InputDeviceList.h"
#include "HighResolutionTime.h"

InputDeviceList *deviceList = NULL;

LRESULT CALLBACK SingleWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (deviceList)
    {
        return deviceList->WndProc(hwnd, msg, wparam, lparam);
    }
    return 1;
}

InputDeviceList::InputDeviceList()
{
    baseWndProc = NULL;
}

int InputDeviceList::HandleRawInput(LPARAM lparam)
{
    RAWINPUT raw;
    UINT size = sizeof(raw);
    // read into buffer
    UINT read = GetRawInputData
        ( (HRAWINPUT)lparam
        , RID_INPUT
        , &raw
        , &size
        , sizeof(RAWINPUTHEADER)
        );
    if (read == (UINT)-1)
    {
        fprintf(stderr, "error getting raw input data");
        return -1;
    }
    if (read > size || read < sizeof(RAWINPUTHEADER))
    {
        fprintf(stderr, "size mismatch %d (expected %d)\n", read, size);
        return -1;
    }
    InputDevice *device = NULL;
    for (unsigned int i = 0; i < devices.size(); i++)
    {
        device = devices[i];
        if (device->IsDevice(raw.header.hDevice))
        {
            break;
        }
    }
    if (!device) throw std::runtime_error("No raw input devices");
    if (raw.header.dwType == RIM_TYPEMOUSE)
    {
        auto move = MouseMovement();
        move.DX = raw.data.mouse.lLastX;
        move.DY = raw.data.mouse.lLastY;
		move.Microseconds = HighResolutionTime::GetTime();
        device->EnqueueMouseMovement(move);
    }
    return 0;
}

LRESULT CALLBACK InputDeviceList::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_INPUT)
    {
        switch (msg)
        {
        case WM_INPUT:
            if (!HandleRawInput(lparam)) break;
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    }
    if (!baseWndProc)
    {
        fprintf(stderr, "!!! something is very wrong (base wndproc missing) !!!\n");
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    else
    {
        return CallWindowProc(baseWndProc, hwnd, msg, wparam, lparam);
    }

    return 0; // return 0 to indicate message was processed
}

void InputDeviceList::InterceptEvents(HWND windowHandle)
{
    deviceList = this;
    baseWndProc = (WNDPROC)SetWindowLongPtr
        ( windowHandle
        , GWLP_WNDPROC
        , (LONG_PTR)SingleWndProc
        );
}
