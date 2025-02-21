CXX = g++
CXXFLAGS = -O3 -march=native -Wall -std=c++11
LDFLAGS = -pthread

SOURCES = FastMath.cpp test_fastmath.cpp benchmark_fastmath.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLES = test_fastmath benchmark_fastmath

all: $(EXECUTABLES)

test_fastmath: FastMath.o test_fastmath.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

benchmark_fastmath: FastMath.o benchmark_fastmath.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

.PHONY: all clean 