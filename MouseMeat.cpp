#include "Common.h"
#include "InputDeviceList.h"
#include "MouseMovement.h"

#define TITLE "MouseMeat"

void OutputThread(InputDeviceList *deviceList)
{
    auto movements = std::vector<MouseMovement>();
    for (;;)
    {
        for (auto device : deviceList->devices)
        {
            movements.clear();
            device->DequeueMouseMovements(movements);
            for (auto movement : movements)
            {
                std::cout << movement.DX << ", " << movement.DY << std::endl;
            }
        }
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hignore, LPSTR lpignore, int nCmdShow)
{
    auto deviceList = InputDevice::GetInputDevices();

    WNDCLASS wc;

    wc.style = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TITLE;

    if (!RegisterClass(&wc))
    {
        std::cerr << "Failed to register window class\n";
        return -1;
    }

    auto hwnd =
        CreateWindow
        (TITLE, // class name
         TITLE, // window name
         0,
         CW_USEDEFAULT, // x
         CW_USEDEFAULT, // y
         0, // width
         0, // height
         NULL, // parent
         NULL, // menu
         hInst, // instance
         NULL); // lpParam

    if (!hwnd)
    {
        std::cerr << "Failed to create (hidden) window\n";
        return -1;
    }

    InputDevice::RegisterMouse(hwnd);
    deviceList->InterceptEvents(hwnd);

    auto outputThread = std::thread(OutputThread, deviceList);

    for (;;)
    {
        MSG msg;
        if (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    outputThread.join();

    delete deviceList;

    return 0;
}
