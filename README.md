## MouseMeat

This is a program which outputs a stream of JSON objects representing:
* Information about the mice connected to the system
* Timestamped mouse movement events

It does this until receiving standard input, then exits.

### Goals

* Accuracy/Precision of data:
  * The events should not be subject to mouse smoothing or software acceleration from the OS.
  * Events should be timestamped precisely, with minimal interference from the performance of the program.

* Ease of use from other programs:
  * The program should have a liberal license allowing worry-free reuse for any purpose.
  * The program should communicate using only standard input and output.
  * The output format should be self-explanatory and easy to parse.
  * The output format should be extensible, with minimal risk of backwards incompatiblity.
  * The output format should be consistent. There should be no options to customize it.
  * Incorrect usage should never cause delayed or inaccurate readings.
  * The program should compile to a single dependency-free binary for easy redistribution.

### How it works

MouseMeat uses the [Windows Raw Input API](https://msdn.microsoft.com/en-us/library/windows/desktop/ms645536(v=vs.85).aspx)
to read mouse events.

It runs two threads using standard C++11 threading APIs. Since the
32-bit MinGW does not currently implement those APIs, I am using
[mingw-std-threads](https://github.com/meganz/mingw-std-threads) to
provide them. Thanks to [Mega Limited](https://github.com/meganz) for
that library!

Its main thread creates a hidden window to register for raw input,
outputs information about the currently attached mice, then loops
checking for new mice and movement events.

When this thread receives a movement event, it timestamps it to
microsecond precision using
[QueryPerformanceCounter](https://msdn.microsoft.com/en-us/library/windows/desktop/ms644904(v=vs.85).aspx)
and pushes it into a buffer.

Meanwhile, an IO thread waits for events to accumulate in the buffer.
When events are available, this thread exchanges the buffer of
accumulated events with an empty one, then writes the accumulated
events as JSON to standard output. Even if this IO thread is very slow
to write events -- perhaps due to blocking writes to standard output
-- the main thread can continue to accurately timestamp and buffer
newly received events.

The IO thread also checks for data in standard input and initiates the
clean shutdown of the program when it is received. This is implemented
by [_kbhit](https://msdn.microsoft.com/en-us/library/58w7c94c.aspx) if
standard input is a console, and by
[PeekNamedPipe](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365779(v=vs.85).aspx)
otherwise.

### How to build

Install MinGW using [mingw-get](https://sourceforge.net/projects/mingw/files/latest/download). Then:

        git clone https://github.com/rspeele/MouseMeat.git
        cd MouseMeat
        make

### Potential improvements

I am not a C++ programmer by day, nor do I frequently choose it for
hobby projects. I use it here in order to work directly with the raw
input API and avoid the latency of GC pauses. The code quality could
be signficantly improved, and I may have made performance or threading
blunders.

Due to the way standard input is checked, MouseMeat does not behave
nicely when its standard input is a redirected file. This use case
makes no sense whatsoever, but the lack of any handling for it is
still a wart.

MouseMeat does not output any information about click events. This
might be desirable for click latency tests.

MouseMeat is currently only available for Windows.
It should be possible to create a compatible program for Linux by
reading from `/dev/input/event*`, and using either
`/sys/bus/usb/devices` or `/proc/bus/input/devices` to obtain other
device metadata. However, this is not a priority.

### License

MouseMeat is released under the MIT license.