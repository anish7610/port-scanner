#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <getopt.h>
#include <sstream>

std::mutex output_mutex;
std::mutex queue_mutex;
std::condition_variable queue_cv;
std::queue<int> port_queue;

// Top 100 commonly used ports
std::vector<int> top_100_ports = {
    20, 21, 22, 23, 25, 53, 67, 68, 69, 80, 110, 123, 135, 137, 138, 139, 143, 161, 162, 179,
    194, 389, 443, 445, 465, 514, 515, 587, 631, 636, 873, 989, 990, 992, 993, 995, 1080, 1194,
    1352, 1433, 1521, 1723, 2049, 2082, 2083, 2181, 2375, 3128, 3306, 3389, 4444, 4500, 4786,
    5000, 5432, 5900, 5984, 6379, 6667, 7001, 8000, 8008, 8080, 8081, 8443, 8888, 9200, 10000
};

// Function to check if a port is open
void scan_port(const std::string& ip, int port) {
    // {
    //     std::lock_guard<std::mutex> lock(output_mutex);
    //     std::cout << "Scanning port " << port << "...\n";
    // }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &target.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&target, sizeof(target)) == 0) {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Port " << port << " is OPEN\n";
    }
    close(sock);
}

// Worker function for scanning ports using a thread pool
void worker(const std::string& ip) {
    while (true) {
        int port;
        {
            if (port_queue.empty()) return;  // ✅ Exit if no ports left

            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [] { return !port_queue.empty(); });
            port = port_queue.front();
            port_queue.pop();
        }
        scan_port(ip, port);
    }
}

// Function to create worker threads
void thread_scan(const std::string& ip, int num_threads) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, ip);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Add ports to the queue
void add_ports_to_queue(const std::vector<int>& ports) {
    for (int port : ports) {
        port_queue.push(port);
    }
}

void add_ports_to_queue(int start, int end) {
    for (int port = start; port <= end; ++port) {
        port_queue.push(port);
    }
}

// Parse comma-separated ports (e.g., "22,80,443")
std::vector<int> parse_comma_separated_ports(const std::string& ports_str) {
    std::vector<int> ports;
    std::stringstream ss(ports_str);
    std::string port;
    while (getline(ss, port, ',')) {
        ports.push_back(std::stoi(port));
    }
    return ports;
}

// Main function
int main(int argc, char* argv[]) {
    std::string target_ip;
    int num_threads = 20;
    bool scan_all = false, scan_top_100 = false;
    int single_port = -1, start_port = -1, end_port = -1;
    std::vector<int> comma_ports;

    struct option long_options[] = {
        {"ip", required_argument, 0, 'i'},
        {"threads", required_argument, 0, 't'},
        {"all", no_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"range", required_argument, 0, 'r'},
        {"top100", no_argument, 0, 'h'},
        {"ports", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "i:t:ap:r:hc:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i':
                target_ip = optarg;
                break;
            case 't':
                num_threads = std::stoi(optarg);
                break;
            case 'a':
                scan_all = true;
                break;
            case 'p':
                single_port = std::stoi(optarg);
                break;
            case 'r': {
                std::string range = optarg;
                size_t pos = range.find('-');
                if (pos != std::string::npos) {
                    start_port = std::stoi(range.substr(0, pos));
                    end_port = std::stoi(range.substr(pos + 1));
                }
                break;
            }
            case 'h':
                scan_top_100 = true;
                break;
            case 'c':
                comma_ports = parse_comma_separated_ports(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " --ip <IP> [--all] [--port <N>] [--range <start-end>] [--top100] [--ports 22,80,443] [--threads <N>]\n";
                return 1;
        }
    }

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <IP> [--all | --top100 | --range start end | --ports 22,80,443]\n";
        return 1;
    }

    if (target_ip.empty()) {
        std::cerr << "Error: Target IP is required.\n";
        return 1;
    }

    if (scan_all) {
        std::cout << "Scanning all 65535 ports...\n\n";
        add_ports_to_queue(1, 65535);
    } else if (single_port != -1) {
        add_ports_to_queue({single_port});
    } else if (scan_top_100) {
        std::cout << "Scanning top 100 common ports...\n\n";
        add_ports_to_queue(top_100_ports);
    } else if (!comma_ports.empty()) {
        add_ports_to_queue(comma_ports);
    } else if (start_port != -1 && end_port != -1) {
        std::cout << "Scanning port range " << start_port << "-" << end_port << "...\n\n";
        add_ports_to_queue(start_port, end_port);
    } else {
        std::cerr << "Usage: " << argv[0] << " --ip <IP> [--all] [--port <N>] [--range <start-end>] [--top100] [--ports 22,80,443] [--threads <N>]\n";
        return 1;
    }

    thread_scan(target_ip, num_threads);
    queue_cv.notify_all();
    
    return 0;
}
