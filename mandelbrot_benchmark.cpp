#include <iostream>
#include <complex>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <sstream>

class MandelbrotRenderer {
private:
    int width, height;
    int max_iterations;
    
    int mandelbrot_iterations(std::complex<double> c) {
        std::complex<double> z = 0;
        for (int i = 0; i < max_iterations; i++) {
            if (std::abs(z) > 2.0) return i;
            z = z * z + c;
        }
        return max_iterations;
    }
    
    std::string get_colored_char(int iterations) {
        if (iterations >= max_iterations) {
            return "\033[40m \033[0m"; // Black background for Mandelbrot set
        }
        
        // Color gradients: Blue -> Cyan -> Green -> Yellow -> Red -> Magenta
        const std::string colors[] = {
            "\033[44m ", // Blue
            "\033[46m ", // Cyan  
            "\033[42m ", // Green
            "\033[43m ", // Yellow
            "\033[41m ", // Red
            "\033[45m "  // Magenta
        };
        
        int color_index = (iterations * 6) / max_iterations;
        if (color_index >= 6) color_index = 5;
        
        return colors[color_index] + "\033[0m";
    }
    
    char get_char(int iterations) {
        if (iterations >= max_iterations) return '#';
        
        // More detailed character gradient
        const char chars[] = " .:-=+*#%@";
        int index = iterations * (sizeof(chars) - 2) / max_iterations;
        return chars[index];
    }
    
public:
    MandelbrotRenderer(int w, int h, int max_iter) 
        : width(w), height(h), max_iterations(max_iter) {}
    
    double render(double x_min, double x_max, double y_min, double y_max, 
                  bool visualize = true, bool progressive = false, bool use_color = false) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (visualize) {
            std::cout << "\033[2J\033[H"; // Clear screen
        }
        
        double x_scale = (x_max - x_min) / width;
        double y_scale = (y_max - y_min) / height;
        
        for (int row = 0; row < height; row++) {
            double y = y_min + row * y_scale;
            
            for (int col = 0; col < width; col++) {
                double x = x_min + col * x_scale;
                std::complex<double> c(x, y);
                
                int iterations = mandelbrot_iterations(c);
                
                if (visualize) {
                    if (use_color) {
                        std::cout << get_colored_char(iterations);
                    } else {
                        std::cout << get_char(iterations);
                    }
                }
            }
            
            if (visualize) {
                std::cout << std::endl;
                if (progressive && row % 2 == 0) {
                    std::cout.flush();
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
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
    std::cout << "Mandelbrot Set Benchmark" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Small render for visualization
    std::cout << "\n1. Visual demonstration (80x40, color):" << std::endl;
    std::cout << "Rendering colorful Mandelbrot view..." << std::endl;
    
    MandelbrotRenderer small_renderer(80, 40, 100);
    double small_time = small_renderer.render(-2.5, 1.0, -1.25, 1.25, true, false, true);
    std::cout << "\nTime: " << small_time << " ms" << std::endl;
    std::cout << "\nPress Enter to continue to benchmark...";
    std::cin.get();
    
    // Different zoom levels for benchmarking
    std::cout << "\n2. Performance benchmark (different zoom levels):" << std::endl;
    
    struct ZoomLevel {
        std::string name;
        double x_min, x_max, y_min, y_max;
        int resolution;
        int iterations;
    };
    
    std::vector<ZoomLevel> zooms = {
        {"Full view", -2.5, 1.0, -1.25, 1.25, 200, 100},
        {"Zoom 1x", -1.0, 0.0, -0.5, 0.5, 200, 150},
        {"Zoom 2x", -0.75, -0.25, -0.25, 0.25, 200, 200},
        {"Deep zoom", -0.7463, -0.7453, 0.1102, 0.1112, 200, 500}
    };
    
    std::vector<double> zoom_times;
    for (const auto& zoom : zooms) {
        std::cout << zoom.name << " (" << zoom.resolution << "x" << zoom.resolution 
                  << ", " << zoom.iterations << " iter) - ";
        std::cout.flush();
        
        MandelbrotRenderer renderer(zoom.resolution, zoom.resolution, zoom.iterations);
        double time = renderer.render(zoom.x_min, zoom.x_max, zoom.y_min, zoom.y_max, false);
        zoom_times.push_back(time);
        
        std::cout << "Time: " << time << " ms" << std::endl;
    }
    
    std::cout << "\n3. Resolution scaling test:" << std::endl;
    std::vector<int> resolutions = {100, 200, 400, 800};
    std::vector<double> resolution_times;
    
    for (int res : resolutions) {
        std::cout << res << "x" << res << " resolution - ";
        std::cout.flush();
        
        MandelbrotRenderer renderer(res, res, 100);
        double time = renderer.render(-2.5, 1.0, -1.25, 1.25, false);
        resolution_times.push_back(time);
        
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
    
    std::cout << "\nMandelbrot Benchmark Results:" << std::endl;
    std::cout << "- Full view (200x200, 100 iter): " << zoom_times[0] << " ms" << std::endl;
    std::cout << "- Zoom 1x (200x200, 150 iter): " << zoom_times[1] << " ms" << std::endl;
    std::cout << "- Zoom 2x (200x200, 200 iter): " << zoom_times[2] << " ms" << std::endl;
    std::cout << "- Deep zoom (200x200, 500 iter): " << zoom_times[3] << " ms" << std::endl;
    std::cout << "- Resolution 100x100: " << resolution_times[0] << " ms" << std::endl;
    std::cout << "- Resolution 200x200: " << resolution_times[1] << " ms" << std::endl;
    std::cout << "- Resolution 400x400: " << resolution_times[2] << " ms" << std::endl;
    std::cout << "- Resolution 800x800: " << resolution_times[3] << " ms" << std::endl;
    
    std::cout << "\nSystem information detected automatically" << std::endl;
    
    return 0;
}