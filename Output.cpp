#include "Output.h"

namespace Output
{
    std::mutex sync;

    void OutputDeviceInfo(const DeviceInfo &device)
    {
        std::lock_guard<std::mutex> lock(sync);
        std::cout
            << "{" << "\"type\"" << ":" << "\"deviceInfo\""
            << "," << "\"deviceInfo\"" << ":"
            << "{" << "\"deviceId\"" << ":" << device.DeviceId
            << "," << "\"usbVendorId\"" << ":" << device.UsbVendorId
            << "," << "\"usbProductId\"" << ":" << device.UsbProductId
            // TODO: should really escape these next two strings...
            << "," << "\"driver\"" << ":" << "\"" << device.Driver << "\""
            << "," << "\"description\"" << ":" << "\"" << device.Description << "\""
            << "}"
            << "}"
            << std::endl;
    }
    void OutputEvent(const Event &event)
    {
        std::lock_guard<std::mutex> lock(sync);
        std::cout
            << "{" << "\"type\"" << ":" << "\"move\""
            << "," << "\"move\"" << ":"
            << "{" << "\"deviceId\"" << ":" << event.DeviceId
            << "," << "\"us\"" << ":" << event.Time
            << "," << "\"dx\"" << ":" << event.DX
            << "," << "\"dy\"" << ":" << event.DY
            << "}"
            << "}"
            << std::endl;
    }
}
