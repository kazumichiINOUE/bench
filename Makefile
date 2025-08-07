CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: wavefront_benchmark mandelbrot_benchmark benchmark_runner

wavefront_benchmark: wavefront_benchmark.cpp
	$(CXX) $(CXXFLAGS) -DCXXFLAGS='"$(CXXFLAGS)"' -o wavefront_benchmark wavefront_benchmark.cpp

mandelbrot_benchmark: mandelbrot_benchmark.cpp
	$(CXX) $(CXXFLAGS) -DCXXFLAGS='"$(CXXFLAGS)"' -o mandelbrot_benchmark mandelbrot_benchmark.cpp

benchmark_runner: benchmark_runner.cpp benchmark_logger.cpp
	$(CXX) $(CXXFLAGS) -DCXXFLAGS='"$(CXXFLAGS)"' -o benchmark_runner benchmark_runner.cpp

clean:
	rm -f wavefront_benchmark mandelbrot_benchmark benchmark_runner benchmark_results.md

.PHONY: clean
