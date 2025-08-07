#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

class BenchmarkLogger {
private:
    std::string results_file;
    
    std::string get_current_datetime() {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
    
    std::string get_system_info_compact() {
        std::stringstream info;
        
        // Get OS info
        #ifdef __APPLE__
            std::system("sw_vers -productVersion > /tmp/os_info.txt 2>/dev/null");
            std::ifstream os_file("/tmp/os_info.txt");
            std::string os_version;
            if (std::getline(os_file, os_version)) {
                info << "macOS " << os_version;
            } else {
                info << "macOS";
            }
            os_file.close();
            std::system("rm -f /tmp/os_info.txt");
        #elif __linux__
            info << "Linux";
        #else
            info << "Unknown";
        #endif
        
        return info.str();
    }
    
    std::string get_cpu_info_compact() {
        std::stringstream info;
        
        #ifdef __APPLE__
            std::system("sysctl -n machdep.cpu.brand_string > /tmp/cpu_info.txt 2>/dev/null");
            std::ifstream cpu_file("/tmp/cpu_info.txt");
            std::string cpu;
            if (std::getline(cpu_file, cpu)) {
                info << cpu;
            } else {
                info << "Unknown CPU";
            }
            cpu_file.close();
            std::system("rm -f /tmp/cpu_info.txt");
        #elif __linux__
            std::system("grep 'model name' /proc/cpuinfo | head -1 | cut -d':' -f2 | sed 's/^ *//' > /tmp/cpu_info.txt 2>/dev/null");
            std::ifstream cpu_file("/tmp/cpu_info.txt");
            std::string cpu;
            if (std::getline(cpu_file, cpu)) {
                info << cpu;
            } else {
                info << "Unknown CPU";
            }
            cpu_file.close();
            std::system("rm -f /tmp/cpu_info.txt");
        #else
            info << "Unknown CPU";
        #endif
        
        return info.str();
    }
    
    std::string get_memory_info_compact() {
        std::stringstream info;
        
        #ifdef __APPLE__
            std::system("sysctl -n hw.memsize | awk '{print $1/1024/1024/1024 \" GB\"}' > /tmp/mem_info.txt 2>/dev/null");
            std::ifstream mem_file("/tmp/mem_info.txt");
            std::string mem;
            if (std::getline(mem_file, mem)) {
                info << mem;
            } else {
                info << "Unknown";
            }
            mem_file.close();
            std::system("rm -f /tmp/mem_info.txt");
        #elif __linux__
            std::system("grep MemTotal /proc/meminfo | awk '{print $2/1024/1024 \" GB\"}' > /tmp/mem_info.txt 2>/dev/null");
            std::ifstream mem_file("/tmp/mem_info.txt");
            std::string mem;
            if (std::getline(mem_file, mem)) {
                info << mem;
            } else {
                info << "Unknown";
            }
            mem_file.close();
            std::system("rm -f /tmp/mem_info.txt");
        #else
            info << "Unknown";
        #endif
        
        return info.str();
    }
    
    bool file_exists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }
    
    void create_header_if_needed() {
        if (!file_exists(results_file)) {
            std::ofstream file(results_file);
            file << "# Benchmark Results\n\n";
            file << "| Date | Machine | OS | CPU | Memory | Compiler | Mandelbrot Full | Mandelbrot Zoom1 | Mandelbrot Zoom2 | Mandelbrot Deep | WaveFront 50x50 | WaveFront 100x100 | WaveFront 200x200 | WaveFront 400x400 |\n";
            file << "|------|---------|----|----|---------|----------|-----------------|------------------|------------------|-----------------|-----------------|-------------------|-------------------|-------------------|\n";
            file.close();
        }
    }
    
    void force_recreate_with_header() {
        std::ofstream file(results_file);
        file << "# Benchmark Results\n\n";
        file << "| Date | Machine | OS | CPU | Memory | Compiler | Mandelbrot Full | Mandelbrot Zoom1 | Mandelbrot Zoom2 | Mandelbrot Deep | WaveFront 50x50 | WaveFront 100x100 | WaveFront 200x200 | WaveFront 400x400 |\n";
        file << "|------|---------|----|----|---------|----------|-----------------|------------------|------------------|-----------------|-----------------|-------------------|-------------------|-------------------|\n";
        file.close();
    }
    
public:
    BenchmarkLogger(const std::string& filename = "benchmark_results.md") 
        : results_file(filename) {}
    
    void log_results(const std::string& machine_name,
                    const std::string& compiler_flags,
                    double mandelbrot_full, double mandelbrot_zoom1, 
                    double mandelbrot_zoom2, double mandelbrot_deep,
                    double wavefront_50, double wavefront_100,
                    double wavefront_200, double wavefront_400) {
        
        std::string temp_file = results_file + ".tmp";
        
        try {
            // Step 1: Read and clean existing content
            std::vector<std::string> valid_lines;
            bool has_header = false;
            bool has_separator = false;
            
            if (file_exists(results_file)) {
                std::ifstream read_file(results_file);
                std::string line;
                
                while (std::getline(read_file, line)) {
                    // Skip empty lines and garbage
                    if (line.empty() || line.find("No newline") != std::string::npos) {
                        continue;
                    }
                    
                    // Keep title, header, separator, and data rows
                    if (line.find("# Benchmark Results") == 0) {
                        valid_lines.push_back(line);
                    } else if (line.find("| Date |") == 0) {
                        valid_lines.push_back(line);
                        has_header = true;
                    } else if (line.find("|------|") == 0) {
                        valid_lines.push_back(line);
                        has_separator = true;
                    } else if (line.find("|") == 0 && line.find(" ms |") != std::string::npos) {
                        // Data rows (start with | and contain " ms |")
                        valid_lines.push_back(line);
                    }
                }
                read_file.close();
            }
            
            // Step 2: Ensure proper header structure
            if (valid_lines.empty() || !has_header || !has_separator) {
                valid_lines.clear();
                valid_lines.push_back("# Benchmark Results");
                valid_lines.push_back("");
                valid_lines.push_back("| Date | Machine | OS | CPU | Memory | Compiler | Mandelbrot Full | Mandelbrot Zoom1 | Mandelbrot Zoom2 | Mandelbrot Deep | WaveFront 50x50 | WaveFront 100x100 | WaveFront 200x200 | WaveFront 400x400 |");
                valid_lines.push_back("|------|---------|----|----|---------|----------|-----------------|------------------|------------------|-----------------|-----------------|-------------------|-------------------|-------------------|");
            }
            
            // Step 3: Create new data row
            std::stringstream new_row;
            new_row << "| " << get_current_datetime()
                    << " | " << machine_name
                    << " | " << get_system_info_compact()
                    << " | " << get_cpu_info_compact()
                    << " | " << get_memory_info_compact()
                    << " | " << compiler_flags
                    << " | " << mandelbrot_full << " ms"
                    << " | " << mandelbrot_zoom1 << " ms"
                    << " | " << mandelbrot_zoom2 << " ms"
                    << " | " << mandelbrot_deep << " ms"
                    << " | " << wavefront_50 << " ms"
                    << " | " << wavefront_100 << " ms"
                    << " | " << wavefront_200 << " ms"
                    << " | " << wavefront_400 << " ms"
                    << " |";
            
            // Step 4: Write to temporary file
            std::ofstream temp_out(temp_file);
            if (!temp_out.is_open()) {
                throw std::runtime_error("Cannot create temporary file");
            }
            
            for (const auto& line : valid_lines) {
                temp_out << line << "\n";
            }
            temp_out << new_row.str() << "\n";
            temp_out.close();
            
            // Step 5: Verify temporary file is valid
            std::ifstream verify(temp_file);
            if (!verify.is_open()) {
                throw std::runtime_error("Cannot verify temporary file");
            }
            
            // Count table rows (should be at least header + separator + 1 data row)
            int table_rows = 0;
            std::string verify_line;
            while (std::getline(verify, verify_line)) {
                if (verify_line.find("|") == 0) {
                    table_rows++;
                }
            }
            verify.close();
            
            if (table_rows < 3) {
                throw std::runtime_error("Temporary file validation failed");
            }
            
            // Step 6: Replace original file with temporary file
            if (std::rename(temp_file.c_str(), results_file.c_str()) != 0) {
                throw std::runtime_error("Cannot replace original file");
            }
            
            std::cout << "\nResults logged to " << results_file << std::endl;
            
        } catch (const std::exception& e) {
            // Cleanup temporary file on error
            std::remove(temp_file.c_str());
            std::cout << "\nError logging results: " << e.what() << std::endl;
            std::cout << "Original file preserved." << std::endl;
        }
    }
};