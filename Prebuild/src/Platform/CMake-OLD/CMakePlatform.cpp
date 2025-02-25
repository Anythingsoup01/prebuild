#include "CMakePlatform.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

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
            ProjectConfig cfg = BuildProjectConfig(str);
            m_Projects.InlineProjects.push_back(cfg);
        }

        for (int i = 0; i < externalProjectCount; i++)
        {
            size_t val = 0;
            std::string str = ParseProject(val, m_ExternalProjectDirs[i]);
            ProjectConfig cfg = BuildProjectConfig(str);
            m_Projects.ExternalProjects.push_back(cfg);
        }

        Build();

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
        size_t pos = 0;
        while (pos != std::string::npos)
        {
            size_t eol = m_WorkspaceString.find_first_of("\r\n", pos);
            std::string line = m_WorkspaceString.substr(pos, eol - pos);

            std::string keyword;
            if (ContainsKeyword(line, keyword))
            {
                if (keyword == "workspace")
                    m_WorkspaceConfig.Name = ParseSingleResponse(keyword.c_str(), line);

                if (keyword == "version")
                    m_WorkspaceConfig.Version = ParseSingleResponse(keyword.c_str(), line);

                if (keyword == "architecture")
                    m_WorkspaceConfig.Architecture = ParseSingleResponse(keyword.c_str(), line);

                if (keyword == "flags")
                    m_WorkspaceConfig.Flags = ParseMultipleResponse(keyword.c_str(), m_WorkspaceString, pos);
            }

            size_t nextLinePos = m_WorkspaceString.find_first_not_of("\r\n", eol);
            size_t nextEOL = m_WorkspaceString.find_first_of("\r\n", nextLinePos);
            if (nextLinePos != std::string::npos)
            {
                pos = nextLinePos;
            }
            else
            {
                break;
            }
            
        }
    }

    CMakePlatform::ProjectConfig CMakePlatform::BuildProjectConfig(std::string& strCache)
    {
        ProjectConfig cfg;
        size_t pos = 0;
        while (pos != std::string::npos)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            std::string keyword;
            if (ContainsKeyword(line, keyword))
            {
                if (keyword == "project")
                    cfg.Name = ParseSingleResponse(keyword.c_str(), line);

                else if (keyword == "mainfile")
                    cfg.MainFileDirectory= ParseSingleResponse(keyword.c_str(), line);

                else if (keyword == "kind")
                    cfg.Kind = ParseSingleResponse(keyword.c_str(), line);

                else if (keyword == "files")
                    cfg.Files = ParseMultipleResponse(keyword.c_str(), strCache, pos);
                
                else if (keyword == "includedirs")
                    cfg.IncludeDirectories = ParseMultipleResponse(keyword.c_str(), strCache, pos);

                else if (keyword == "links")
                    cfg.Links = ParseMultipleResponse(keyword.c_str(), strCache, pos);

            }
            
            size_t nextLinePos = strCache.find_first_not_of("\r\n", eol);
            size_t nextEOL = strCache.find_first_of("\r\n", nextLinePos);
            if (nextLinePos != std::string::npos)
            {
                pos = nextLinePos;
            }
            else
            {
                break;
            }
            
        }
        return cfg;
    }

    bool CMakePlatform::ContainsKeyword(std::string& line, std::string& outKeyword)
    {
        for(auto keyword : Keywords)
        {
            if (line.find(keyword) != std::string::npos)
            {
                outKeyword = keyword;
                return true;
            }
        }
        return false;
    }

    bool CMakePlatform::ContainsPathKeyword(std::string& line, std::string& outKeyword)
    {
        for(auto keyword : PathKeywords)
        {
            if (line.find(keyword) != std::string::npos)
            {
                outKeyword = keyword;
                return true;
            }
        }
        return false;
    }

    std::string CMakePlatform::GetCMakeSyntax(std::string& keyword)
    {
        if (keyword == "$(ROOTDIR)")
            return "${CMAKE_SOURCE_DIR}";
        return "";
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
        res.erase(remove_if(res.begin(), res.end(), isspace), res.end());
        res.erase(0, keyLen);
        res.erase(std::remove(res.begin(), res.end(), '"'), res.end());

        return res;
    }
    std::vector<std::string> CMakePlatform::ParseMultipleResponse(const char* keyword, std::string& strCache, size_t& pos)
    {
        size_t linePos = strCache.find_first_of(keyword, pos);
        if (linePos == std::string::npos)
        {
            std::stringstream ss;
            ss << "Syntax error - '" << keyword << "'";
            Utils::PrintError(ss.str().c_str());
            return {};
        }
        std::vector<std::string> out;
        linePos = strCache.find_first_of("{", linePos);
        while (linePos != std::string::npos)
        {
            size_t eol = strCache.find_first_of("\r\n", linePos);
            std::string str = strCache.substr(linePos, eol - linePos);
            str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
            if (str.find("{") != std::string::npos)
            {
                goto JUMP;
            }
                
            if (str.find("}") != std::string::npos)
            {
                return out;
            }

            if (str.find_first_of("\"") == str.find_last_of("\""))
            {
                Utils::PrintError("Syntax error ' \" '");
                return {};
            }
            if (str.find_first_of(",") != str.find_last_of(",") || str.find(",") == std::string::npos)
            {
                Utils::PrintError("Syntax error ' , '");
                return {};
            }

            str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
            str.erase(std::remove(str.begin(), str.end(), ','), str.end());
            out.push_back(str);
JUMP:
            size_t nextLinePos = strCache.find_first_not_of("\r\n", eol);
            if (nextLinePos == std::string::npos)
            {
                Utils::PrintError("Syntax error: ' } '");
                return {};
            }
            linePos = nextLinePos;
        }
        Utils::PrintError("OUT OF SCOPE!");
        return {};
    }

    
    void CMakePlatform::Build()
    {
        std::ofstream out("CMakeLists.txt");
        if (!out.is_open())
        {
            Utils::PrintError("Could not generate CMakeLists.txt");
            return;
        }

        std::stringstream ss;
        ss << BuildWorkspace() << "\n";

        for (auto& cfg : m_Projects.InlineProjects)
        {
            ss << BuildProject(cfg) << "\n";
        }

        for (auto& cfg : m_Projects.ExternalProjects)
        {
            ss << "add_subdirectory(" << cfg.Name << ")\n";
            std::stringstream outDir;
            outDir << cfg.Name << "/CMakeLists.txt";
            std::ofstream out(outDir.str());
            if (!out.is_open())
            {
                Utils::PrintError("Coould not generate external CMakeLists.txt");
                return;
            }
            std::stringstream ss;
            ss << BuildProject(cfg) << "\n";
            out << ss.str();
            out.close();
        }
        out << ss.str();
        out.close();
    }
    std::string CMakePlatform::BuildWorkspace()
    {
        std::stringstream ss;
        ss << "cmake_minimum_required(VERSION " << m_WorkspaceConfig.Version << ")\n\n"
           << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n"
           << "project(" << m_WorkspaceConfig.Name << ")\n\n";
        return ss.str();
    }
    std::string CMakePlatform::BuildProject(ProjectConfig& cfg)
    {
        std::stringstream ss;

        if (!cfg.Files.empty())
        {
            ss << "set(SRCS\n";
            for (auto& src : cfg.Files)
            {
                std::string keyword;
                if (ContainsPathKeyword(src, keyword))
                {
                    std::string out = src;
                    out.erase(0, keyword.length());
                    ss << "    " << GetCMakeSyntax(keyword) << out << std::endl;
                }
                else
                {
                    ss << "    " << src << std::endl;
                }
            }

            ss << ")\n\n";
        }

        if (cfg.Kind == "StaticLib")
        {
            ss << "add_library(" << cfg.Name << " STATIC " << cfg.MainFileDirectory << " ${SRCS})\n\n";
        }
        else if (cfg.Kind == "SharedLib")
        {
            ss << "add_library(" << cfg.Name << " SHARED " << cfg.MainFileDirectory << " ${SRCS})\n\n";
        }
        else if (cfg.Kind == "ConsoleApp")
        {
            ss << "add_executable(" << cfg.Name << " " << cfg.MainFileDirectory << " ${SRCS})\n\n";
        }
        else
        {
            Utils::PrintError("Syntax error - 'Kind'");
            return "";
        }

        if (!cfg.IncludeDirectories.empty())
        {
            ss << "target_include_directories(" << cfg.Name <<  " PRIVATE\n";
            for (auto& dir : cfg.IncludeDirectories)
            {
                std::string keyword;
                if (ContainsPathKeyword(dir, keyword))
                {
                    std::string out = dir;
                    out.erase(0, keyword.length());
                    ss << "    " << GetCMakeSyntax(keyword) << out << std::endl;
                }
                else
                {
                    ss << "    " << dir << std::endl;
                }
            }

            ss << ")\n\n";
        }

        if (!cfg.Links.empty())
        {
            ss << "target_link_libraries(" << cfg.Name << std::endl;
            for (auto& link : cfg.Links)
            {
                ss << "    " << link << std::endl;
            }
            ss << ")\n\n";
        }

        return ss.str();
    }
}
