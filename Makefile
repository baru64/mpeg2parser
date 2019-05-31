src = $(wildcard src/*.cpp)
obj = $(src:.c=.o)

LDFLAGS = -Wall --std=c++11

mpeg2parser: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)
