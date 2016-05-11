CXX=g++
CXXFLAGS=-std=c++11
LIBS=-lm -lstdc++

OBJ= \
	HighResolutionTime.o \
	InputDevice.o \
	InputDeviceList.o \
	MouseMeat.o

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

all: $(OBJ)
	$(CXX) -o MouseMeat.exe $(CXXFLAGS) $(LIBS) $^

.PHONY: clean

clean:
	rm -f *.o
