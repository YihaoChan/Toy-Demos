CXX = g++
CXXFLAGS = -W -Wall

target = ping
srcs = $(wildcard *.cpp)
headers = $(wildcard *.h)

.PHONY: all
all: $(target)

.PHONY: FORCE
FORCE:

$(target): $(headers) FORCE
	$(CXX) $(srcs) -o $@ -g

clean:
	rm -f $(target)