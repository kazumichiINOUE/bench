#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <sstream>

class WaveFrontPlanner {
private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> distance;
    int width, height;
    
public:
    WaveFrontPlanner(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<int>(width, 0));
        distance.resize(height, std::vector<int>(width, -1));
        
        // Generate maze-like obstacles
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i == 0 || i == height-1 || j == 0 || j == width-1) {
                    grid[i][j] = 1; // walls at borders
                } else if ((i % 4 == 2) && (j % 4 == 2)) {
                    grid[i][j] = 1; // scattered obstacles
                }
            }
        }
    }
    
    void displayGrid() {
        std::cout << "\033[2J\033[H"; // Clear screen and move cursor to top
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (grid[i][j] == 1) {
                    std::cout << "█";
                } else if (distance[i][j] == -1) {
                    std::cout << " ";
                } else {
                    char c = '0' + (distance[i][j] % 10);
                    if (distance[i][j] >= 10) c = 'A' + ((distance[i][j] - 10) % 6);
                    std::cout << c;
                }
            }
            std::cout << std::endl;
        }
        std::cout.flush();
    }
    
    double planPath(int startX, int startY, int goalX, int goalY, bool visualize = true) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Reset distance grid
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
            
            if (visualize) {
                displayGrid();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
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
        
        if (visualize) {
            displayGrid();
            std::cout << "\nPath planning completed!" << std::endl;
            std::cout << "Path length from start to goal: " << distance[startY][startX] << std::endl;
            
            // Show optimal path from goal to start
            if (distance[startY][startX] != -1) {
                std::cout << "\nTracing optimal path (goal to start)..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                // Build complete path first
                std::vector<std::pair<int, int>> path;
                int curr_x = startX, curr_y = startY;
                path.push_back({curr_x, curr_y});
                
                while (curr_x != goalX || curr_y != goalY) {
                    int dx[] = {-1, 1, 0, 0};
                    int dy[] = {0, 0, -1, 1};
                    
                    for (int i = 0; i < 4; i++) {
                        int nx = curr_x + dx[i];
                        int ny = curr_y + dy[i];
                        
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                            distance[ny][nx] != -1 && distance[ny][nx] < distance[curr_y][curr_x]) {
                            curr_x = nx;
                            curr_y = ny;
                            path.push_back({curr_x, curr_y});
                            break;
                        }
                    }
                }
                
                // Animate path highlighting
                for (size_t step = 0; step < path.size(); step++) {
                    std::cout << "\033[2J\033[H";
                    for (int i = 0; i < height; i++) {
                        for (int j = 0; j < width; j++) {
                            bool on_path = false;
                            bool current_step = false;
                            
                            // Check if this position is on the path and already highlighted
                            for (size_t p = 0; p <= step; p++) {
                                if (path[p].first == j && path[p].second == i) {
                                    on_path = true;
                                    if (p == step) current_step = true;
                                    break;
                                }
                            }
                            
                            if (grid[i][j] == 1) {
                                std::cout << "█";
                            } else if (on_path) {
                                if (current_step) {
                                    std::cout << "\033[1;33m*\033[0m"; // Bright yellow for current
                                } else {
                                    std::cout << "\033[1;32m#\033[0m"; // Bright green for path
                                }
                            } else if (distance[i][j] == -1) {
                                std::cout << " ";
                            } else {
                                // Dim the background numbers
                                char c = '0' + (distance[i][j] % 10);
                                if (distance[i][j] >= 10) c = 'A' + ((distance[i][j] - 10) % 6);
                                std::cout << "\033[2m" << c << "\033[0m"; // Dim
                            }
                        }
                        std::cout << std::endl;
                    }
                    std::cout << "Path step " << (step + 1) << "/" << path.size() 
                              << " at (" << path[step].first << "," << path[step].second << ")" << std::endl;
                    std::cout.flush();
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                std::cout << "\nOptimal path completed!" << std::endl;
            }
        }
        
        return duration.count() / 1000.0; // Return time in milliseconds
    }
};

std::string get_system_info() {
    std::stringstream info;
    
    // Get OS info
    #ifdef __APPLE__
        info << "OS: macOS ";
        std::system("sw_vers -productVersion > /tmp/macos_version.txt 2>/dev/null");
        std::ifstream version_file("/tmp/macos_version.txt");
        std::string version;
        if (std::getline(version_file, version)) {
            info << version;
        }
        version_file.close();
        std::system("rm -f /tmp/macos_version.txt");
        info << "\n";
    #elif __linux__
        info << "OS: Linux ";
        std::system("uname -r > /tmp/linux_version.txt 2>/dev/null");
        std::ifstream version_file("/tmp/linux_version.txt");
        std::string version;
        if (std::getline(version_file, version)) {
            info << version;
        }
        version_file.close();
        std::system("rm -f /tmp/linux_version.txt");
        info << "\n";
    #else
        info << "OS: Unknown\n";
    #endif
    
    // Get CPU info
    #ifdef __APPLE__
        info << "CPU: ";
        std::system("sysctl -n machdep.cpu.brand_string > /tmp/cpu_info.txt 2>/dev/null");
        std::ifstream cpu_file("/tmp/cpu_info.txt");
        std::string cpu;
        if (std::getline(cpu_file, cpu)) {
            info << cpu;
        }
        cpu_file.close();
        std::system("rm -f /tmp/cpu_info.txt");
        info << "\n";
    #elif __linux__
        info << "CPU: ";
        std::system("grep 'model name' /proc/cpuinfo | head -1 | cut -d':' -f2 | sed 's/^ *//' > /tmp/cpu_info.txt 2>/dev/null");
        std::ifstream cpu_file("/tmp/cpu_info.txt");
        std::string cpu;
        if (std::getline(cpu_file, cpu)) {
            info << cpu;
        }
        cpu_file.close();
        std::system("rm -f /tmp/cpu_info.txt");
        info << "\n";
    #else
        info << "CPU: Unknown\n";
    #endif
    
    // Get memory info
    #ifdef __APPLE__
        info << "Memory: ";
        std::system("sysctl -n hw.memsize | awk '{print $1/1024/1024/1024 \" GB\"}' > /tmp/mem_info.txt 2>/dev/null");
        std::ifstream mem_file("/tmp/mem_info.txt");
        std::string mem;
        if (std::getline(mem_file, mem)) {
            info << mem;
        }
        mem_file.close();
        std::system("rm -f /tmp/mem_info.txt");
        info << "\n";
    #elif __linux__
        info << "Memory: ";
        std::system("grep MemTotal /proc/meminfo | awk '{print $2/1024/1024 \" GB\"}' > /tmp/mem_info.txt 2>/dev/null");
        std::ifstream mem_file("/tmp/mem_info.txt");
        std::string mem;
        if (std::getline(mem_file, mem)) {
            info << mem;
        }
        mem_file.close();
        std::system("rm -f /tmp/mem_info.txt");
        info << "\n";
    #else
        info << "Memory: Unknown\n";
    #endif
    
    return info.str();
}

int main() {
    std::cout << "WaveFront Planner Benchmark" << std::endl;
    std::cout << "===========================" << std::endl;
    
    // Small grid for visualization
    std::cout << "\n1. Visual demonstration (30x15 grid):" << std::endl;
    WaveFrontPlanner small_planner(30, 15);
    double small_time = small_planner.planPath(1, 1, 28, 13, true);
    std::cout << "Time: " << small_time << " ms" << std::endl;
    
    std::cout << "\nPress Enter to continue to benchmark...";
    std::cin.get();
    
    // Large grid for benchmarking
    std::cout << "\n2. Performance benchmark:" << std::endl;
    std::vector<int> sizes = {50, 100, 200, 400};
    std::vector<double> benchmark_times;
    
    for (int size : sizes) {
        std::cout << "Grid size: " << size << "x" << size << " - ";
        std::cout.flush();
        
        WaveFrontPlanner planner(size, size);
        double time = planner.planPath(1, 1, size-2, size-2, false);
        benchmark_times.push_back(time);
        
        std::cout << "Time: " << time << " ms" << std::endl;
    }
    
    // Output formatted results for copy-paste
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "BENCHMARK RESULTS (Copy-Paste Format)" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Get system info
    std::string system_info = get_system_info();
    std::cout << system_info;
    std::cout << "Compiler: g++ -std=c++17 -Wall" << std::endl;
    std::cout << "Date: " << __DATE__ << std::endl;
    
    std::cout << "\nWaveFront Planner Benchmark Results:" << std::endl;
    std::cout << "- Grid 50x50: " << benchmark_times[0] << " ms" << std::endl;
    std::cout << "- Grid 100x100: " << benchmark_times[1] << " ms" << std::endl;
    std::cout << "- Grid 200x200: " << benchmark_times[2] << " ms" << std::endl;
    std::cout << "- Grid 400x400: " << benchmark_times[3] << " ms" << std::endl;
    std::cout << "- Visual demo time: " << small_time << " ms" << std::endl;
    
    std::cout << "\nSystem information detected automatically" << std::endl;
    
    return 0;
}
