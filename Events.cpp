#include "Events.h"

namespace Events
{
    class StaticData
    {
        std::vector<Event> buffer1, buffer2;
    public:
        std::mutex sync;
        std::vector<Event> *inactive, *active;

        StaticData()
        {
            active = &buffer1;
            inactive = &buffer2;
        }
    };

    StaticData data;

    std::vector<Event> &SwapBuffer()
    {
        data.sync.lock();

        auto previous = data.active;
        data.active = data.inactive;
        data.inactive = previous;

        data.sync.unlock();

        return *previous;
    }

    void Buffer(Event event)
    {
        data.sync.lock();

        data.active->push_back(event);

        data.sync.unlock();
    }
}
