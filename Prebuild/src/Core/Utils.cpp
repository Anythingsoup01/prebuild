#include "Utils.h"

#include <fstream>
#include <iostream>

namespace Utils
{
    System GetSystem(char* sysString)
    {
        std::string systemString(sysString);
        if (systemString == "cmake")
            return System::CMAKE;

        return System::NONE;
    }
    void PrintError(const char* msg)
    {
        printf("ERROR: %s\n", msg);
    }
    void PrintError(const std::string& msg)
    {
        PrintError(msg.c_str());
    }
    void PrintError(const std::stringstream& msg)
    {
        std::string msgStr = msg.str();
        PrintError(msgStr.c_str());
    }

    void PrintWarning(const char* msg)
    {
        printf("WARNING: %s\n", msg);
    }
    void PrintWarning(const std::string& msg)
    {
        PrintWarning(msg.c_str());
    }
    void PrintWarning(const std::stringstream& msg)
    {
        std::string msgStr = msg.str();
        PrintWarning(msgStr.c_str());
    }

    std::string GetStringFromFile(const std::string filePath)
    {
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            PrintError("File could not be open");
            return std::string();
        }
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    int GetCharacterCount(const std::string& line, const char character)
    {
        const char* lineCSTR = line.c_str();
        int count = 0;
        for (int i = 0; i  < line.length(); i++)
        {
            if (lineCSTR[i] == character)
            {
                count++;
            }
        }
        return count;

    }
}
