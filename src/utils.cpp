#include "utils.hpp"
#include <iostream>
#include <string>

void logInfo(const std::string& message) {
    std::cerr << "[INFO] " << message << std::endl;
}

void logWarning(const std::string& message) {
    std::cerr << "[WARNING] " << message << std::endl;
}

void logError(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}