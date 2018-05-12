.PHONY: all clean

CXX=c++
CXXFLAGS += -O2 -Wall -std=c++11 -pthread
LDFLAGS +=

TARGET=pstats

all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $^ $(LDFLAGS)

clean:
	rm -fr *.o $(TARGET)
