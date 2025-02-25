#pragma once

#include <string>
#include <sstream>

namespace Utils
{
    void PrintError(const char* msg);
    void PrintError(const std::string& msg);
    void PrintError(const std::stringstream& msg);

    void PrintWarning(const char* msg);
    void PrintWarning(const std::string& msg);
    void PrintWarning(const std::stringstream& msg);

    std::string GetStringFromFile(const std::string filePath);
    int GetCharacterCount(const std::string& line, const char character);
}
