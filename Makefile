CXX=g++
CXXFLAGS=-std=c++11 -Wall
LIBS=-lm -lstdc++

DEPS = \
	Common.h \
	HighResolutionTime.h \
	InputDevice.h \
	InputDeviceList.h \
	MouseMovement.h

OBJ= \
	HighResolutionTime.o \
	InputDevice.o \
	InputDeviceList.o \
	MouseMeat.o

%.o: %.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

all: $(OBJ)
	$(CXX) -o MouseMeat.exe $(CXXFLAGS) $(LIBS) $^

.PHONY: clean

clean:
	rm -f *.o
