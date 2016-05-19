#include "Input.h"
#include "Output.h"
#include "Events.h"
#include <conio.h>

namespace Input
{
    // We don't allow these characters in a device description or driver string.
    // If they appear, we'll replace them with underscores.
    // This is just a lazy way to avoid escaping the output JSON.
    const char *blacklist = "\\\"\r\n";

#define BASEENUM "SYSTEM\\CurrentControlSet\\Enum\\"

    // Open registry key for reading corresponding to
    // the device with RIDI_DEVICENAME of name.
    //
    // Return 0 on success, -1 on failure.
    int openDeviceKey(const char *name, HKEY *handle, int32_t &vendorId, int32_t &productId)
    {
        static char regpath[512];
        const char *base = BASEENUM;
        while (*name && (*name == '\\' || *name == '?')) name++;
        uint32_t i, k, p;
        for (i = 0; base[i] && i < sizeof(regpath); i++)
        {
            regpath[i] = base[i];
        }
        for (k = 0, p = 0; name[k] && i < sizeof(regpath); (void)(++i && ++k))
        {
            char c = name[k];
            if (c == '#')
            {
                if (++p > 2) break;
                regpath[i] = '\\';
            }
            else regpath[i] = c;
        }
        regpath[i] = '\0';
        int vid, pid;
        if (2 == sscanf(regpath, BASEENUM "HID\\VID_%x&PID_%x", &vid, &pid))
        {
            vendorId = vid;
            productId = pid;
        }
        auto open = RegOpenKeyEx
            ( HKEY_LOCAL_MACHINE
            , regpath
            , 0
            , KEY_READ
            , handle
            );
        return open == ERROR_SUCCESS ? 0 : -1;
    }

    void fillInfo(HANDLE device, const char *key, std::vector<char> &name, int32_t &vendorId, int32_t &productId)
    {
        name.resize(16);
        static char dname[512];
        UINT len = sizeof(dname);
        UINT info = GetRawInputDeviceInfo
            ( device
            , RIDI_DEVICENAME
            , &dname
            , &len
            );
        if (info == (UINT)-1)
        {
            throw std::runtime_error("Couldn't get device info");
        }
        HKEY reg;
        if (openDeviceKey(dname, &reg, vendorId, productId) < 0)
        {
            throw std::runtime_error("Couldn't open device registry key");
        }
        auto status = ERROR_MORE_DATA;
        while (status == ERROR_MORE_DATA)
        {
            DWORD type;
            ULONG size = name.capacity();
            status = RegQueryValueExA
                ( reg
                , key
                , NULL
                , &type
                , (BYTE *)&name[0]
                , &size
                );
            if (status == ERROR_MORE_DATA)
            {
                name.resize(size);
            }
            else
            {
                name[size - 1] = 0;
                for (unsigned i = 0; i < size - 1; i++)
                {
                    for (unsigned j = 0; blacklist[j]; j++)
                    {
                        if (name[i] == blacklist[j]) name[i] = '_';
                    }
                }
            }
        }

        RegCloseKey(reg);
    }

    std::map<HANDLE, int16_t> deviceIds;
    int16_t previousDeviceId = 0;

    int16_t registerDevice(HANDLE hDevice)
    {
        auto existing = deviceIds.find(hDevice);
        if (existing != deviceIds.end())
        {
            return existing->second;
        }
        std::vector<char> driver, descr;
        DeviceInfo info;
        fillInfo(hDevice, "DeviceDesc", descr, info.UsbVendorId, info.UsbProductId);
        fillInfo(hDevice, "Driver", driver, info.UsbVendorId, info.UsbProductId);
        info.DeviceId = ++previousDeviceId;
        info.Driver = &driver[0];
        info.Description = &descr[0];
        Output::OutputDeviceInfo(info);
        deviceIds[hDevice] = info.DeviceId;
        return info.DeviceId;
    }

    void registerDevices()
    {
        PRAWINPUTDEVICELIST rids;
        UINT numrids;
        UINT list = GetRawInputDeviceList(NULL, &numrids, sizeof(RAWINPUTDEVICELIST));
        if (list == (UINT)-1) throw std::runtime_error("Raw input not supported");
        rids = (PRAWINPUTDEVICELIST)(new RAWINPUTDEVICELIST[numrids]);
        list = GetRawInputDeviceList(rids, &numrids, sizeof(RAWINPUTDEVICELIST));
        if (list == (UINT)-1)
        {
            delete[] rids;
            throw std::runtime_error("Couldn't get raw input devices");
        }
        for (UINT i = 0; i < numrids; i++)
        {
            registerDevice(rids[i].hDevice);
        }
        delete[] rids;
    }

    void handleRawInput(LPARAM lparam)
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
            std::cerr << "Error getting raw input data\n";
            return;
        }
        if (read > size || read < sizeof(RAWINPUTHEADER))
        {
            std::cerr << "Size mismatch %d (expected %d)\n";
            return;
        }
        if (raw.header.dwType == RIM_TYPEMOUSE)
        {
            Event ev;
            ev.DeviceId = registerDevice(raw.header.hDevice);
            ev.DX = raw.data.mouse.lLastX;
            ev.DY = raw.data.mouse.lLastY;
            ev.Time = HighResolutionTime::GetTime();
            Events::Buffer(ev);
        }
    }

#define GIDC_ARRIVAL 1
#define GIDC_REMOVAL 2

    void handleDeviceChange(WPARAM wparam, LPARAM lparam)
    {
        HANDLE hDevice = (HANDLE)lparam;
        switch (wparam)
        {
        case GIDC_ARRIVAL:
            registerDevice(hDevice);
            break;
        case GIDC_REMOVAL: break; // TODO: output something?
        }
    }


    WNDPROC baseWndProc;

    LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_INPUT:
            handleRawInput(lparam);
            break;
        case WM_INPUT_DEVICE_CHANGE:
            handleDeviceChange(wparam, lparam);
            break;
        }
        if (!baseWndProc)
        {
            std::cerr << "!!! something is very wrong (base wndproc missing) !!!\n";
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
        else
        {
            return CallWindowProc(baseWndProc, hwnd, msg, wparam, lparam);
        }

        return 0; // return 0 to indicate message was processed
    }

    void overrideWndProc(HWND window)
    {
        baseWndProc = (WNDPROC)SetWindowLongPtr
            ( window
            , GWLP_WNDPROC
            , (LONG_PTR)wndProc
            );
    }

#define USAGE_PAGE_GENERIC_DESKTOP 0x01
#define USAGE_MOUSE 0x02
#define USAGE_KEYBOARD 0x06

    void StartListening(HWND window)
    {
        static bool registered = false;
        if (registered) return;
        registered = true;
        static RAWINPUTDEVICE rid;
        rid.usUsagePage = USAGE_PAGE_GENERIC_DESKTOP;
        rid.usUsage = USAGE_MOUSE;
        rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
        rid.hwndTarget = window;
        RegisterRawInputDevices(&rid, 1, sizeof(rid));
        registerDevices();
        overrideWndProc(window);
    }

    struct WinInput
    {
        HANDLE handle;
        bool isConsole;
    };

    WinInput GetWinInput()
    {
        WinInput result;
        result.handle = GetStdHandle(STD_INPUT_HANDLE);
        result.isConsole = GetFileType(result.handle) == FILE_TYPE_CHAR;
        return result;
    }

    bool HasStandardInput()
    {
        static auto winInput = GetWinInput();
        // Crappy thing about Windows:
        // we can't just tell if stdin has input,
        // we have to do different stuff depending on whether
        // it's the console (_kbhit()) or a pipe.
        if (winInput.isConsole) return _kbhit();
        DWORD avail;
        auto success = PeekNamedPipe
            ( winInput.handle
            , NULL
            , 0
            , NULL
            , &avail
            , NULL
            );
        if (!success) throw std::runtime_error("Failed to peek input");
        return avail > 0;
    }
}
