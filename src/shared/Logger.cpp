#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace medical::shared {

class Logger {
public:
    static void info(const std::string& message) {
        log("INFO", message);
    }
    
    static void error(const std::string& message) {
        log("ERROR", message);
    }
    
    static void warn(const std::string& message) {
        log("WARN", message);
    }
    
private:
    static void log(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                  << " [" << level << "] " << message << std::endl;
    }
};

} // namespace medical::shared
