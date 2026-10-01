CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude

SRCS = src/Location.cpp \
       src/Edge.cpp \
       src/Graph.cpp \
       src/Dijkstra.cpp \
       src/RoutePlanner.cpp \
       src/DemandSimulator.cpp \
       src/Profiler.cpp \
       src/Visualizer.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = smart_transit
TEST_TARGET = test_runner

.PHONY: all run test clean help

all: $(TARGET)

$(TARGET): $(OBJS) src/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(OBJS) tests/test_routing.o
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $^
	./$(TEST_TARGET)

tests/test_routing.o: tests/test_routing.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET) *.svg *.dot

help:
	@echo "Smart City Public Transit System (C++17) - Makefile Commands"
	@echo "------------------------------------------------------------"
	@echo "make        : Compile the binary executable"
	@echo "make run    : Compile and launch the interactive transit system"
	@echo "make test   : Compile and run algorithmic unit tests"
	@echo "make clean  : Remove compiled binaries and object files"
