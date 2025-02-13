#include "CMakePlatform.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

namespace Utils
{
    void PrintError(const char* msg)
    {
        printf("ERROR: %s\n", msg);
    }
}

namespace Prebuild
{
    CMakePlatform::CMakePlatform()
    {
        std::ifstream in("prebuild.lua");
        if (!in.is_open())
        {
            Utils::PrintError("Could not open prebuild!");
            return;
        }

        std::stringstream ss;
        ss << in.rdbuf();
        in.close();

        int inlineProjectCount = 0;
        int externalProjectCount = 0;

        size_t pos = 0;

        std::string line;
        while(getline(ss, line))
        {
            if (line.find("project") != std::string::npos)
            {
                if (line.find("startproject") == std::string::npos)
                    inlineProjectCount++;
            }
            else if (line.find("external") != std::string::npos)
            {
                externalProjectCount++;
                std::string dir = line;
                dir.erase(0, 9);
                dir.erase(std::remove(dir.begin(), dir.end(), '"'), dir.end());
                dir.erase(std::remove(dir.begin(), dir.end(), ')'), dir.end());
                m_ExternalProjectDirs.push_back(dir);
            }
        }
        m_PrebuildString = ss.str();
        m_WorkspaceString = ParseWorkspace(pos);
        std::cout << m_WorkspaceString << std::endl;
        
        for (int i = 0; i < inlineProjectCount; i++)
        {
            std::string str = ParseProject(pos);
            m_InlineProjectStrings.push_back(str);
            std::cout << m_InlineProjectStrings[i] << std::endl;
        }

        for (int i = 0; i < externalProjectCount; i++)
        {
            size_t val = 0;
            std::string str = ParseProject(val, m_ExternalProjectDirs[i]);
            m_ExternalProjectStrings.push_back(str);
            std::cout << m_ExternalProjectStrings[i] << std::endl;
        }

        
    }

    std::string CMakePlatform::ParseWorkspace(size_t& pos)
    {
        std::string file = m_PrebuildString;
        std::stringstream workspaceStr;
        while (pos != std::string::npos)
        {
            size_t eol = file.find_first_of("\r\n", pos);
            std::string subStr = file.substr(pos, eol - pos);

            workspaceStr << subStr << std::endl;

            size_t nextLinePos = file.find_first_not_of("\r\n", eol);
            size_t nextEOL = file.find_first_of("\r\n", nextLinePos);
            if (nextLinePos != std::string::npos)
            {
                std::string tmp = file.substr(nextLinePos, nextEOL - nextLinePos);
                if (tmp.find("project") != std::string::npos || tmp.find("external") != std::string::npos)
                {
                    pos = nextLinePos;
                    return workspaceStr.str();
                }
            }
            pos = nextLinePos;

        }
        return "";
    }

    std::string CMakePlatform::ParseProject(size_t& pos, std::string dir)
    {
        std::stringstream ss;
        if (dir.empty())
        {
            ss << m_PrebuildString;
        }
        else
        {
            std::stringstream fileDir;
            fileDir << dir << "/prebuild.lua";
            std::ifstream in(fileDir.str());
            if (!in.is_open())
            {
                Utils::PrintError("Could not open prebuild!");
                return "";
            }
            ss << in.rdbuf();
            in.close();
        }
        std::string file = ss.str();
        std::stringstream projectStr;
        while (pos != std::string::npos)
        {
            size_t eol = file.find_first_of("\r\n", pos);
            std::string subStr = file.substr(pos, eol - pos);

            std::cout << subStr << std::endl;
            projectStr << subStr << std::endl;

            size_t nextLinePos = file.find_first_not_of("\r\n", eol);
            size_t nextEOL = file.find_first_of("\r\n", nextLinePos);
            if (nextLinePos != std::string::npos)
            {
                std::string tmp = file.substr(nextLinePos, nextEOL - nextLinePos);
                if (tmp.find("project") != std::string::npos || tmp.find("external") != std::string::npos)
                {
                    pos = nextLinePos;
                    std::cout << "PROJECT STRING: \n" << projectStr.str() << std::endl;
                    return projectStr.str();
                }
            }
            pos = nextLinePos;

        }
    }

}