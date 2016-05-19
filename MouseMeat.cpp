#include "Common.h"
#include "Events.h"
#include "Input.h"
#include "Output.h"

const char *TITLE = "MouseMeat";

void OutputThread(bool *stop, DWORD uiThread)
{
    try
    {
        while (!Input::HasStandardInput())
        {
            auto events = Events::SwapBuffer();
            for (auto event : events)
            {
                Output::OutputEvent(event);
            }
        }
    }
    catch (const std::exception &err)
    {
        std::cerr << err.what() << std::endl;
    }
    *stop = true;
    PostThreadMessage(uiThread, WM_QUIT, 0, 0);
}

void Run(HINSTANCE hInst)
{
    // Disable syncing iostreams with C stdio,
    // since we won't use C stdio here and it
    // makes iostreams noticeably slow.
    std::ios_base::sync_with_stdio(false);

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
        throw std::runtime_error("Failed to register window class");
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
        throw std::runtime_error("Failed to create (hidden) window");
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
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hignore, LPSTR lpignore, int nCmdShow)
{
    try
    {
        Run(hInst);
        return 0;
    }
    catch (const std::exception &err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
