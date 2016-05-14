#include "Output.h"

namespace Output
{
    void OutputDeviceInfo(const DeviceInfo &device)
    {
        std::cout
            << "{" << "\"type\"" << ":" << "\"deviceInfo\""
            << "," << "\"info\"" << ":"
            << "{" << "\"deviceId\"" << ":" << device.DeviceId
            << "," << "\"usbVendorId\"" << ":" << device.UsbVendorId
            << "," << "\"usbProductId\"" << ":" << device.UsbProductId
            // TODO: should really escape these next two strings...
            << "," << "\"driver\"" << ":" << "\"" << device.Driver << "\""
            << "," << "\"description\"" << ":" << "\"" << device.Description << "\""
            << "}"
            << "}";
    }
    void OutputEvent(const Event &event)
    {
        std::cout
            << "{" << "\"type\"" << ":" << "\"move\""
            << "," << "\"info\"" << ":"
            << "{" << "\"deviceId\"" << ":" << event.DeviceId
            << "," << "\"time\"" << ":" << event.Time
            << "," << "\"dx\"" << ":" << event.DX
            << "," << "\"dy\"" << ":" << event.DY
            << "}"
            << "}";
    }
}
