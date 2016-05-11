#include "Common.h"
#include "InputDevice.h"
#include "InputDeviceList.h"

const char *blacklist = "\\\"";

// Open registry key for reading corresponding to
// the device with RIDI_DEVICENAME of name.
//
// Return 0 on success, -1 on failure.
int opendevicekey(const char *name, HKEY *handle, int &vendorId, int &productId)
{
    static char regpath[512];
    const char *base = "SYSTEM\\CurrentControlSet\\Enum\\";
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
	if (2 == sscanf(regpath, "SYSTEM\\CurrentControlSet\\Enum\\HID\\VID_%x&PID_%x", &vid, &pid))
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

void fillinfo(HANDLE device, const char *key, std::vector<BYTE> &name, int &vendorId, int &productId)
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
    if (opendevicekey(dname, &reg, vendorId, productId) < 0)
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
            , &name[0]
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
				for (auto j = 0; blacklist[j]; j++)
				{
					if (name[i] == blacklist[j]) name[i] = '_';
				}
			}
        }
    }

    RegCloseKey(reg);
}

InputDeviceList *InputDevice::GetInputDevices()
{
    auto devices = new InputDeviceList();
    PRAWINPUTDEVICELIST rids;
    UINT numrids;
    UINT list = GetRawInputDeviceList(NULL, &numrids, sizeof(RAWINPUTDEVICELIST));
    if (list == (UINT)-1) throw std::runtime_error("Raw input not supported");
    rids = (PRAWINPUTDEVICELIST)malloc(numrids * sizeof(RAWINPUTDEVICELIST));
    if (!rids) throw std::runtime_error("Couldn't allocate memory");
    list = GetRawInputDeviceList(rids, &numrids, sizeof(RAWINPUTDEVICELIST));
    if (list == (UINT)-1)
    {
        free(rids);
        throw std::runtime_error("Couldn't get raw input devices");
    }
    for (UINT i = 0; i < numrids; i++)
    {
        auto dev = new InputDevice();
        dev->handle = rids[i].hDevice;
        dev->type = rids[i].dwType;
        dev->deviceDesc = new std::vector<BYTE>();
		dev->driver = new std::vector<BYTE>();
        dev->moves = new std::queue<MouseMovement>();
        dev->mutex = new std::mutex();
		dev->UsbVendorId = 0;
		dev->UsbProductId = 0;
        fillinfo(dev->handle, "DeviceDesc", *dev->deviceDesc, dev->UsbVendorId, dev->UsbProductId);
		fillinfo(dev->handle, "Driver", *dev->driver, dev->UsbVendorId, dev->UsbProductId);
        devices->devices.push_back(dev);
    }
    free(rids);
    return devices;
}

void InputDevice::EnqueueMouseMovement(MouseMovement movement)
{
    mutex->lock();
    moves->push(movement);
    mutex->unlock();
}

void InputDevice::DequeueMouseMovements(std::vector<MouseMovement> &movements)
{
    mutex->lock();
    while (moves->size())
    {
        movements.push_back(moves->front());
        moves->pop();
    }
    mutex->unlock();
}

bool InputDevice::IsDevice(HANDLE deviceHandle)
{
    return handle == deviceHandle;
}

#define USAGE_PAGE_GENERIC_DESKTOP 0x01
#define USAGE_MOUSE 0x02
#define USAGE_KEYBOARD 0x06

void InputDevice::RegisterMouse(HWND window)
{
    static RAWINPUTDEVICE rid;
    rid.usUsagePage = USAGE_PAGE_GENERIC_DESKTOP;
    rid.usUsage = USAGE_MOUSE;
    rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
    rid.hwndTarget = window;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

const char *InputDevice::DeviceDesc()
{
	return (const char *)&((*deviceDesc)[0]);
}

const char *InputDevice::Driver()
{
	return (const char *)&((*driver)[0]);
}

InputDevice::~InputDevice()
{
    delete deviceDesc;
	delete driver;
    delete moves;
    delete mutex;
}
