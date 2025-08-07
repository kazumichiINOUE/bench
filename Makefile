CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: wavefront_benchmark mandelbrot_benchmark

wavefront_benchmark: wavefront_benchmark.cpp
	$(CXX) $(CXXFLAGS) -o wavefront_benchmark wavefront_benchmark.cpp

mandelbrot_benchmark: mandelbrot_benchmark.cpp
	$(CXX) $(CXXFLAGS) -o mandelbrot_benchmark mandelbrot_benchmark.cpp

clean:
	rm -f wavefront_benchmark mandelbrot_benchmark

.PHONY: clean
