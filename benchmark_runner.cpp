#include "benchmark_logger.cpp"
#include "gist_manager.cpp"
#include <iostream>
#include <string>

// Include benchmark classes
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <sstream>

// WaveFrontPlanner class (simplified for runner)
class WaveFrontPlanner {
private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> distance;
    int width, height;
    
public:
    WaveFrontPlanner(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<int>(width, 0));
        distance.resize(height, std::vector<int>(width, -1));
        
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i == 0 || i == height-1 || j == 0 || j == width-1) {
                    grid[i][j] = 1;
                } else if ((i % 4 == 2) && (j % 4 == 2)) {
                    grid[i][j] = 1;
                }
            }
        }
    }
    
    double planPath(int startX, int startY, int goalX, int goalY) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (auto& row : distance) {
            std::fill(row.begin(), row.end(), -1);
        }
        
        std::queue<std::pair<int, int>> queue;
        queue.push({goalY, goalX});
        distance[goalY][goalX] = 0;
        
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};
        
        while (!queue.empty()) {
            auto [y, x] = queue.front();
            queue.pop();
            
            for (int i = 0; i < 4; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                
                if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                    grid[ny][nx] == 0 && distance[ny][nx] == -1) {
                    distance[ny][nx] = distance[y][x] + 1;
                    queue.push({ny, nx});
                }
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

// MandelbrotRenderer class (simplified for runner)
class MandelbrotRenderer {
private:
    int width, height, max_iterations;
    
    int mandelbrot_iterations(std::complex<double> c) {
        std::complex<double> z = 0;
        for (int i = 0; i < max_iterations; i++) {
            if (std::abs(z) > 2.0) return i;
            z = z * z + c;
        }
        return max_iterations;
    }
    
public:
    MandelbrotRenderer(int w, int h, int max_iter) 
        : width(w), height(h), max_iterations(max_iter) {}
    
    double render(double x_min, double x_max, double y_min, double y_max) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        double x_scale = (x_max - x_min) / width;
        double y_scale = (y_max - y_min) / height;
        
        for (int row = 0; row < height; row++) {
            double y = y_min + row * y_scale;
            for (int col = 0; col < width; col++) {
                double x = x_min + col * x_scale;
                std::complex<double> c(x, y);
                mandelbrot_iterations(c);
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

int main() {
    std::cout << "Automated Benchmark Runner" << std::endl;
    std::cout << "===========================" << std::endl;
    
    // Get configuration
    std::string machine_name;
    std::cout << "Enter machine name (e.g., 'MacBook Pro M3'): ";
    std::getline(std::cin, machine_name);
    
    std::string gist_id;
    std::cout << "Enter existing Gist ID (leave empty for new): ";
    std::getline(std::cin, gist_id);
    
    std::string github_token;
    std::cout << "Enter GitHub token (leave empty for anonymous): ";
    std::getline(std::cin, github_token);
    
    // Setup Gist manager
    GistManager gist_manager(gist_id, github_token);
    
    // Download existing results if Gist ID provided
    if (!gist_id.empty()) {
        gist_manager.download_existing_gist();
    }
    
    std::cout << "\nRunning benchmarks..." << std::endl;
    
    // Run Mandelbrot benchmarks
    std::cout << "Running Mandelbrot benchmarks..." << std::endl;
    
    MandelbrotRenderer renderer1(200, 200, 100);
    double mandelbrot_full = renderer1.render(-2.5, 1.0, -1.25, 1.25);
    
    MandelbrotRenderer renderer2(200, 200, 150);
    double mandelbrot_zoom1 = renderer2.render(-1.0, 0.0, -0.5, 0.5);
    
    MandelbrotRenderer renderer3(200, 200, 200);
    double mandelbrot_zoom2 = renderer3.render(-0.75, -0.25, -0.25, 0.25);
    
    MandelbrotRenderer renderer4(200, 200, 500);
    double mandelbrot_deep = renderer4.render(-0.7463, -0.7453, 0.1102, 0.1112);
    
    // Run WaveFront benchmarks
    std::cout << "Running WaveFront benchmarks..." << std::endl;
    
    WaveFrontPlanner planner1(50, 50);
    double wavefront_50 = planner1.planPath(1, 1, 48, 48);
    
    WaveFrontPlanner planner2(100, 100);
    double wavefront_100 = planner2.planPath(1, 1, 98, 98);
    
    WaveFrontPlanner planner3(200, 200);
    double wavefront_200 = planner3.planPath(1, 1, 198, 198);
    
    WaveFrontPlanner planner4(400, 400);
    double wavefront_400 = planner4.planPath(1, 1, 398, 398);
    
    // Get compiler flags
    #ifndef CXXFLAGS
    #define CXXFLAGS "Unknown"
    #endif
    std::string compiler_flags = "g++ " + std::string(CXXFLAGS);
    
    // Log results
    BenchmarkLogger logger;
    logger.log_results(machine_name, compiler_flags,
                      mandelbrot_full, mandelbrot_zoom1, mandelbrot_zoom2, mandelbrot_deep,
                      wavefront_50, wavefront_100, wavefront_200, wavefront_400);
    
    std::cout << "\nBenchmark completed!" << std::endl;
    std::cout << "Results saved to benchmark_results.md" << std::endl;
    
    // Upload to Gist
    std::cout << "\nUploading results to GitHub Gist..." << std::endl;
    if (gist_manager.upload_to_gist("Cross-Platform Benchmark Results")) {
        std::cout << "Upload successful!" << std::endl;
        if (gist_id.empty() && !gist_manager.get_gist_id().empty()) {
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "★ GIST ID FOR OTHER MACHINES: " << gist_manager.get_gist_id() << std::endl;
            std::cout << "★ Copy this ID to use on other computers!" << std::endl;
            std::cout << std::string(50, '=') << std::endl;
        }
    } else {
        std::cout << "Upload failed. Results are still saved locally." << std::endl;
    }
    
    gist_manager.cleanup_temp_files();
    
    return 0;
}
