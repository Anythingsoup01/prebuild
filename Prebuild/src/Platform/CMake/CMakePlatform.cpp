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
        BuildWorkspaceConfig();
        //return;
        for (int i = 0; i < inlineProjectCount; i++)
        {
            std::string str = ParseProject(pos);
            m_InlineProjectStrings.push_back(str);
        }

        for (int i = 0; i < externalProjectCount; i++)
        {
            size_t val = 0;
            std::string str = ParseProject(val, m_ExternalProjectDirs[i]);
            m_ExternalProjectStrings.push_back(str);
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
        if (!dir.empty())
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
        else
        {
            ss << m_PrebuildString;
        }

        std::string file = ss.str();
        std::stringstream projectStr;
        pos = file.find_first_of("p", pos);
        while (pos != std::string::npos)
        {
            size_t eol = file.find_first_of("\r\n", pos);
            std::string subStr = file.substr(pos, eol - pos);

            projectStr << subStr << std::endl;

            size_t nextLinePos = file.find_first_not_of("\r\n", eol);
            size_t nextEOL = file.find_first_of("\r\n", nextLinePos);
            if (nextLinePos != std::string::npos)
            {
                std::string tmp = file.substr(nextLinePos, nextEOL - nextLinePos);
                if (tmp.find("project") != std::string::npos || tmp.find("external") != std::string::npos)
                {
                    pos = nextLinePos;
                    return projectStr.str();
                }
                pos = nextLinePos;
            }
            else
            {
                return projectStr.str();
            }

        }
        return "Failed to compile";
    }

    void CMakePlatform::BuildWorkspaceConfig()
    {
        size_t pos = m_WorkspaceString.find_first_of("workspace");
        while (pos != std::string::npos)
        {
            size_t eol = m_WorkspaceString.find_first_of("\r\n", pos);
            std::string line = m_WorkspaceString.substr(pos, eol - pos);

            if (ContainsKeyword(line))
            {
                if (line.find("workspace") != std::string::npos)
                {
                    m_WorkspaceConfig.Name = ParseSingleResponse("workspace", line);
                    std::cout << m_WorkspaceConfig.Name << std::endl;
                }
            }

            
        }
    }

    CMakePlatform::ProjectConfig CMakePlatform::BuildProjectConfig(int& index)
    {

    }

    bool CMakePlatform::ContainsKeyword(std::string& line)
    {
        for(auto keyword : Keywords)
        {
            if (line.find(keyword) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::string CMakePlatform::ParseSingleResponse(const char* keyword, std::string& line)
    {
        if (line.find(keyword) == std::string::npos)
        {
            Utils::PrintError("KEYWORD DOESN'T MATCH!");
            return "";
        }

        std::string key(keyword);
        size_t keyLen = key.length();
        std::string res = line;
        res.erase(0, keyLen);
        res.erase(std::remove(res.begin(), res.end(), '"'), res.end());
        res.erase(remove_if(res.begin(), res.end(), isspace), res.end());

        return res;
    }

}