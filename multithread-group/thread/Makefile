CXX = g++
CXXFLAGS = -W -Wall
LDFLAGS = -pthread
CPP_FLAGS= -std=c++11

target = main
srcs = $(wildcard *.cpp)
headers = $(wildcard *.h)

.PHONY: all
all: $(target)

.PHONY: FORCE
FORCE:

$(target): $(headers) FORCE
	$(CXX) $(srcs) -o $@ $(LDFLAGS) $(CPP_FLAGS)

clean:
	rm -f $(target)