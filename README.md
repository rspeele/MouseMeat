## MouseMeat

This is a program which outputs a stream of JSON objects representing:
* Information about the mice connected to the system
* Timestamped mouse movement events

It does this until receiving standard input, then exits.

### Downloading

You can obtain a compiled binary from the [releases page](https://github.com/rspeele/MouseMeat/releases).

### Output format

MouseMeat outputs one JSON object, representing a single event, per
line. This was chosen over outputting a JSON list of event objects,
because in other programs it is easier to handle streaming data from
MouseMeat's redirected standard output when you can simply read the
stream a line at a time and feed each line into your JSON parser.

Each event has a type field, and an object field with the same name as
the value of its type field, which carries type-specific data.

Currently, there are only two types of event. Here are examples to
show the format -- remember though that in practice each event would
be formatted on a single line.

#### Device Info

Each mouse device plugged in when MouseMeat starts generates a device
info event. These events are also generated when additional mice are
plugged in while MouseMeat is running.

The `deviceId` field is an arbitrary number chosen by MouseMeat to
correlate the device with its movement events within a single log. It
has nothing to do with the type of mouse and cannot be correlated
across logs!

The `usbVendorId` and `usbProductId` fields, taken together, usually
identify a mouse model, but some manufacturers use the same product ID
for multiple models. There is nothing like a serial number that
identifies a specific instance of a product.

```json
{ "type": "deviceInfo"
, "deviceInfo":
  { "deviceId": 1
  , "usbVendorId": 2
  , "usbProductId": 3
  , "driver": "An identifier string for the driver this mouse is using"
  , "description": "The description of the device according to Windows"
  }
}
```

#### Move

Each movement reported by a mouse generates a move event. Note that
the `us` field corresponds to the microseconds elapsed as of the
event, since an undefined epoch. This does not tell you in calendar
time when the log was recorded, but can be used to measure the time
between two events in the same log. You should use a 64-bit integer to
represent this field in your application!

The `dx` and `dy` fields tell you how far the mouse moved in this
event on the X and Y axes. A positive value on the X axis means the
mouse moved to the right. A positive value on the Y axis means the
mouse moved forward (away from the user).

```json
{ "type": "move"
, "move":
  { "deviceId": 1
  , "us": 60000000
  , "dx": -1
  , "dy": 3
  }
}
```

#### Custom events

If you would like to write a program that processes these logs and
saves extra metadata, like computed statistics or info about the mouse
driver settings in use, I suggest making up your own event type and
adding it at the start or end of the event stream rather than creating
a completely new format.

No built-in MouseMeat event type will ever start with the string
`"custom"`. Therefore, you can safely name your own event types
something like `"customMyAppMyEventType"`.

### Project goals

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