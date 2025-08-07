#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>

class GistManager {
private:
    std::string gist_id;
    std::string github_token;
    std::string filename;
    
    bool execute_command(const std::string& command) {
        int result = std::system(command.c_str());
        return result == 0;
    }
    
public:
    GistManager(const std::string& id = "", const std::string& token = "", 
                const std::string& file = "benchmark_results.md") 
        : gist_id(id), github_token(token), filename(file) {}
    
    void set_gist_id(const std::string& id) {
        gist_id = id;
    }
    
    void set_github_token(const std::string& token) {
        github_token = token;
    }
    
    bool download_existing_gist() {
        if (gist_id.empty()) {
            std::cout << "No Gist ID provided, creating new local file." << std::endl;
            return false;
        }
        
        std::string command = "curl -s https://api.github.com/gists/" + gist_id + 
                             " | python3 -c \"import sys, json; data=json.load(sys.stdin); print(list(data['files'].values())[0]['content'])\" > " + filename;
        
        std::cout << "Downloading existing Gist..." << std::endl;
        if (execute_command(command)) {
            // Check if file has content
            std::ifstream file(filename);
            if (file.good() && file.peek() != std::ifstream::traits_type::eof()) {
                std::cout << "Successfully downloaded existing results." << std::endl;
                return true;
            }
        }
        
        std::cout << "Could not download existing Gist or it was empty." << std::endl;
        return false;
    }
    
    bool upload_to_gist(const std::string& description = "Benchmark Results") {
        if (!std::ifstream(filename).good()) {
            std::cout << "Error: " << filename << " not found!" << std::endl;
            return false;
        }
        
        // Escape content for JSON
        std::string temp_file = "/tmp/gist_content.txt";
        std::string escape_command = "cat " + filename + " | sed 's/\"/\\\\\"/g' | sed 's/$/\\\\n/' | tr -d '\\n' > " + temp_file;
        execute_command(escape_command);
        
        // Read escaped content
        std::ifstream content_file(temp_file);
        std::string content;
        std::getline(content_file, content);
        content_file.close();
        
        // Create JSON payload
        std::string json_payload = "{"
            "\"description\":\"" + description + "\","
            "\"public\":true,"
            "\"files\":{"
                "\"" + filename + "\":{"
                    "\"content\":\"" + content + "\""
                "}"
            "}"
        "}";
        
        // Write JSON to temp file
        std::string json_file = "/tmp/gist_payload.json";
        std::ofstream payload_file(json_file);
        payload_file << json_payload;
        payload_file.close();
        
        std::string command;
        if (gist_id.empty()) {
            // Create new Gist
            if (github_token.empty()) {
                // Anonymous Gist (no updates possible)
                command = "curl -X POST -H \"Content-Type: application/json\" "
                         "-d @" + json_file + " https://api.github.com/gists";
            } else {
                // Authenticated Gist
                command = "curl -X POST -H \"Authorization: token " + github_token + "\" "
                         "-H \"Content-Type: application/json\" "
                         "-d @" + json_file + " https://api.github.com/gists";
            }
        } else {
            // Update existing Gist
            if (github_token.empty()) {
                std::cout << "Error: GitHub token required to update existing Gist!" << std::endl;
                return false;
            }
            command = "curl -X PATCH -H \"Authorization: token " + github_token + "\" "
                     "-H \"Content-Type: application/json\" "
                     "-d @" + json_file + " https://api.github.com/gists/" + gist_id;
        }
        
        std::cout << "Uploading to GitHub Gist..." << std::endl;
        std::string output_file = "/tmp/gist_response.json";
        command += " > " + output_file + " 2>&1";
        
        if (execute_command(command)) {
            // Parse response to get Gist URL
            std::string extract_url = "grep '\"html_url\"' " + output_file + 
                                    " | head -1 | sed 's/.*\"html_url\":\"//;s/\".*$//'";
            std::string url_command = extract_url + " > /tmp/gist_url.txt";
            execute_command(url_command);
            
            std::ifstream url_file("/tmp/gist_url.txt");
            std::string url;
            if (std::getline(url_file, url) && !url.empty()) {
                std::cout << "Successfully uploaded to: " << url << std::endl;
                
                // If this was a new Gist, extract the ID for future updates
                if (gist_id.empty()) {
                    std::string extract_id = "grep '\"id\"' " + output_file + 
                                           " | head -1 | sed 's/.*\"id\":\"//;s/\".*$//'";
                    std::string id_command = extract_id + " > /tmp/gist_id.txt";
                    execute_command(id_command);
                    
                    std::ifstream id_file("/tmp/gist_id.txt");
                    std::string new_id;
                    if (std::getline(id_file, new_id) && !new_id.empty()) {
                        gist_id = new_id;
                        std::cout << "\n" << std::string(50, '=') << std::endl;
                        std::cout << "★ IMPORTANT: New Gist ID: " << gist_id << std::endl;
                        std::cout << "★ Save this ID for future updates on other machines!" << std::endl;
                        std::cout << std::string(50, '=') << std::endl;
                    }
                }
                
                return true;
            }
        }
        
        std::cout << "Failed to upload to Gist. Check your internet connection and credentials." << std::endl;
        return false;
    }
    
    void cleanup_temp_files() {
        execute_command("rm -f /tmp/gist_content.txt /tmp/gist_payload.json /tmp/gist_response.json /tmp/gist_url.txt /tmp/gist_id.txt");
    }
    
    std::string get_gist_id() const {
        return gist_id;
    }
};