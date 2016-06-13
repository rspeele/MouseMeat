CXX=g++
CXXFLAGS=-std=c++11 -Wall -O3 -static -static-libgcc
LIBS=-lm -lstdc++

DEPS = \
	Common.h \
	HighResolutionTime.h \
	Input.h \
	Output.h \
	Events.h

OBJ= \
	HighResolutionTime.o \
	Input.o \
	Output.o \
	Events.o \
	MouseMeat.o

%.o: %.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

all: $(OBJ)
	$(CXX) -o MouseMeat.exe $(CXXFLAGS) $(LIBS) $^

.PHONY: clean

clean:
	rm -f *.o
