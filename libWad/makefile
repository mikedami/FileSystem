# libWad makefile

CXX = g++

CXXFLAGS = -std=c++11 -Wall -Wextra

LIBRARY = libWad.a
SOURCES = Wad.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -f $(LIBRARY)

.PHONY: all clean