#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {
	std::string trim(const std::string& value);
	std::string getCurrentDate();
	int readInt(const std::string& prompt);
	double readDouble(const std::string& prompt);
	std::string readLine(const std::string& prompt);
}

#endif
