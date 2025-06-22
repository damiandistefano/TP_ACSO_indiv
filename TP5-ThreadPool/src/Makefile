# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -g

# Default target
all: test-main

# Compilar con main.cc
test-main: thread-pool.cc Semaphore.cc main.cc
	$(CXX) $(CXXFLAGS) -o test-main thread-pool.cc Semaphore.cc main.cc

# Compilar con tptest.cc (test oficial)
test-oficial: thread-pool.cc Semaphore.cc tptest.cc
	$(CXX) $(CXXFLAGS) -o test-oficial thread-pool.cc Semaphore.cc tptest.cc

# Compilar con tpcustomtest.cc (test alternativo)
test-custom: thread-pool.cc Semaphore.cc tpcustomtest.cc
	$(CXX) $(CXXFLAGS) -o test-custom thread-pool.cc Semaphore.cc tpcustomtest.cc

# Clean build artifacts
clean:
	rm -f test-main test-oficial test-custom
