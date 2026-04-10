#include "../include/Utils.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace Utils {
    std::string trim(const std::string& value) {
        std::size_t start = 0;
        while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
            ++start;
        }

        if (start == value.size()) {
            return "";
        }

        std::size_t end = value.size() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(value[end]))) {
            --end;
        }

        return value.substr(start, end - start + 1);
    }

    std::string getCurrentDate() {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);

        std::ostringstream out;
        out << std::put_time(localTime, "%Y-%m-%d");
        return out.str();
    }

    int readInt(const std::string& prompt) {
        while (true) {
            std::cout << prompt;
            int value;
            if (std::cin >> value) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cout << "Invalid integer. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    double readDouble(const std::string& prompt) {
        while (true) {
            std::cout << prompt;
            double value;
            if (std::cin >> value) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cout << "Invalid number. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        return trim(value);
    }
}