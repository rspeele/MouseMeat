#include "Common.h"
#include "Events.h"
#include "Input.h"
#include "Output.h"

const char *TITLE = "MouseMeat";

void OutputThread(bool *stop, DWORD uiThread)
{
    char cmd;
    while (std::cin >> cmd)
    {
        for (;;)
        {
            auto events = Events::SwapBuffer();
            for (auto event : events)
            {
                Output::OutputEvent(event);
            }
            if (events.size() > 0) break;
        }
        if (cmd == 'q')
        {
            *stop = true;
            break;
        }
    }
    PostThreadMessage(uiThread, WM_QUIT, 0, 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hignore, LPSTR lpignore, int nCmdShow)
{
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

    Input::StartListening(hwnd);

    bool stop = false;
    auto uiThread = GetCurrentThreadId();
    auto outputThread = std::thread(OutputThread, &stop, uiThread);

    while (!stop)
    {
        MSG msg;
        if (GetMessage(&msg, NULL, 0, 0) > 0 && !stop)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    outputThread.join();

    return 0;
}
