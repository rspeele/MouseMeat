#include "Events.h"

namespace Events
{
    class StaticData
    {
        std::vector<Event> buffer1, buffer2;
    public:
        std::mutex sync;
        std::condition_variable signal;
        std::vector<Event> *inactive, *active;
        const std::vector<Event> empty;

        StaticData()
        {
            active = &buffer1;
            inactive = &buffer2;
        }
    };

    StaticData data;

    const std::vector<Event> &SwapBuffer()
    {
        std::unique_lock<std::mutex> lock(data.sync);
        auto wait = data.signal.wait_for(lock, std::chrono::milliseconds(250));
        if (wait == std::cv_status::timeout)
        {
            return data.empty;
        }
        else
        {
            auto previous = data.active;

            data.active = data.inactive;
            data.active->clear();

            data.inactive = previous;

            return *previous;
        }
    }

    void Buffer(Event event)
    {
        {
            std::lock_guard<std::mutex> lock(data.sync);
            data.active->push_back(event);
        }
        data.signal.notify_one();
    }
}
