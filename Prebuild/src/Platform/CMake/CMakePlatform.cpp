#include "CMakePlatform.h"
#include "Core/Platform.h"
#include "Core/Utils.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

#define PB_PRINT(x) //printf("FUNCTION - %s\n", x)

namespace Prebuild
{

    CMakePlatform::CMakePlatform(const std::string& version)
    {
        PB_PRINT("Constructor");
        m_Version = version;
        m_RootPrebuildString = Utils::GetStringFromFile("prebuild.lua");
        if (m_RootPrebuildString.empty())
        {
            Utils::PrintError("Could not open/find root prebuild file!");
            return;
        }
        if (!CheckSyntax(m_RootPrebuildString))
        {
            return;
        }
        size_t pos = 0;
        m_WorkspaceString = ParseWorkspace(pos);
        if (m_WorkspaceString.empty())
        {
            return;
        }
        BuildWorkspaceConfig();

        while (pos != NPOS)
        {
            size_t eol = m_RootPrebuildString.find_first_of("\r\n", pos);
            std::string line = m_RootPrebuildString.substr(pos, eol - pos);
            ProjectType res = CheckProjectType(line);

            switch (res)
            {
                case ProjectType::INLINE:
                {
                    std::string projStr = ParseProject(pos, std::string());
                    if (projStr.empty())
                    {
                        return;
                    }
                    ProjectConfig cfg = BuildProjectConfig(projStr);
                    m_Projects.InlineProjects.push_back(cfg);
                    break;
                }
                case ProjectType::EXTERNAL:
                {
                    std::string dir = line;

                    dir.erase(remove_if(dir.begin(), dir.end(), isspace), dir.end());
                    dir.erase(std::remove(dir.begin(), dir.end(), '\"'), dir.end());
                    dir.erase(0, 8);

                    std::string projStr = ParseProject(pos, dir);
                    if (projStr.empty())
                    {
                        return;
                    }

                    if(!CheckSyntax(projStr))
                    {
                        return;
                    }

                    ProjectConfig cfg = BuildProjectConfig(projStr);
                    m_Projects.ExternalProjects.emplace(dir, cfg);
                    break;
                }
                default:
                {
                    break;
                }
            }
            pos = m_RootPrebuildString.find_first_not_of("\r\n", eol);
        }
        Build();

    }

    // PARSING FILES ------------------------------------------------------------------------------------

    std::string CMakePlatform::ParseWorkspace(size_t& outPos)
    {
        PB_PRINT("ParseWorkspace");
        std::string file = m_RootPrebuildString;
        std::stringstream out;
        size_t pos = file.find_first_of("workspace", outPos);
        while (pos != NPOS)
        {
            size_t eol = file.find_first_of("\r\n", pos);
            std::string line = file.substr(pos, eol - pos);

            out << line << std::endl;

            size_t nextLinePos = file.find_first_not_of("\r\n", eol);
            if (nextLinePos != NPOS)
            {
                size_t nextEOL = file.find_first_of("\r\n", nextLinePos);
                std::string nextLine = file.substr(nextLinePos, nextEOL - nextLinePos);

                if (nextLine.find("project") != NPOS || nextLine.find("external") != NPOS)
                {
                    outPos = nextLinePos;
                    break;
                }
            }
            else
            {
                Utils::PrintError("No projects found!");
                return std::string();
            }
            pos = nextLinePos;
        }
        return out.str();
    }

    std::string CMakePlatform::ParseProject(size_t& outPos, std::string dir)
    {
        std::stringstream ss;
        if (!dir.empty())
        {
            std::stringstream dirss;
            dirss << dir << "/prebuild.lua";
            std::ifstream in(dirss.str());
            if (!in.is_open())
            {
                std::stringstream oss;
                oss << "Could not open prebuild file at - " << dir;
                Utils::PrintError(oss);
                return std::string();
            }
            ss << in.rdbuf();
            in.close();

            size_t eol = m_RootPrebuildString.find_first_of("\r\n", outPos);
            outPos = m_RootPrebuildString.find_first_not_of("\r\n", eol);

            return ss.str();
        }

        size_t pos = outPos;
        while (pos != NPOS)
        {
            size_t eol = m_RootPrebuildString.find_first_of("\r\n", pos);
            std::string line = m_RootPrebuildString.substr(pos, eol - pos);

            ss << line << std::endl;

            size_t nextLinePos = m_RootPrebuildString.find_first_not_of("\r\n", eol);
            if (nextLinePos != NPOS)
            {
                size_t nextEOL = m_RootPrebuildString.find_first_of("\r\n", nextLinePos);
                std::string nextLine = m_RootPrebuildString.substr(nextLinePos, nextEOL - nextLinePos);
                if (nextLine.find("project") != NPOS || nextLine.find("external") != NPOS)
                {
                    outPos = nextLinePos;
                    break;
                }
                pos = nextLinePos;
            }
            else
                break;
        }

        return ss.str();
    }

    // BUILDING CONFIGS ------------------------------------------------------------------------------------

    void CMakePlatform::BuildWorkspaceConfig()
    {
        std::string file = m_WorkspaceString;
        size_t pos = 0;
        WorkspaceConfig cfg;

        while (pos != NPOS)
        {
            size_t eol = file.find_first_of("\r\n", pos);
            std::string line = file.substr(pos, eol - pos);

            std::string keyword;
            if (ContainsKeyword(line, keyword))
            {
                if (keyword == "workspace")
                    cfg.Name = ParseField(line, keyword);
                if (keyword == "architecture")
                    cfg.Architecture = StringToArchitectureType(ParseField(line, keyword));
                if (keyword == "configurations")
                {
                    if (IsSetForMultipleParameters(file, pos))
                        cfg.Configurations = ParseMultipleFields(file, pos, keyword);
                    else
 ;                      cfg.Configurations.push_back(ParseField(line, keyword));
                }
                if (keyword == "defines")
                {
                    if (IsSetForMultipleParameters(file, pos))
                        cfg.Defines = ParseMultipleFields(file, pos, keyword);
                    else
                        cfg.Defines.push_back(ParseField(line, keyword));
                }
            }

            pos = file.find_first_not_of("\r\n", eol);
            if (pos == NPOS)
                break;
        }
        m_WorkspaceConfig = cfg;
    }

    CMakePlatform::ProjectConfig CMakePlatform::BuildProjectConfig(const std::string& strCache)
    {
        ProjectConfig cfg;
        size_t pos = 0;
        while (pos != NPOS)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            std::string keyword;
            if (ContainsKeyword(line, keyword))
            {
                if (keyword == "project")
                    cfg.Name = ParseField(line, keyword);
                if (keyword == "language")
                    cfg.Language = StringToLanguageType(ParseField(line, keyword));
                if (keyword == "dialect")
                    cfg.Dialect = ParseField(line, keyword);
                if (keyword == "kind")
                    cfg.Kind = StringToKindType(ParseField(line, keyword));
                if (keyword == "includedirs")
                    cfg.IncludedDirectories = ParseMultipleFields(strCache, pos, keyword);
                if (keyword == "files")
                    cfg.Files = ParseMultipleFields(strCache, pos, keyword);
            }

            pos = strCache.find_first_not_of("\r\n", eol);
            if (pos == NPOS)
                break;
        }
        return cfg;
    }


    // BUILDING CMAKELISTS -----------------------------------------------------------------------

    void CMakePlatform::Build()
    {
        std::stringstream ss;

        bool containsC = false;
        bool containsCXX = false;

        for (auto& cfg : m_Projects.InlineProjects)
        {
            if (cfg.Language == LanguageType::C)
                containsC = true;
            if (cfg.Language == LanguageType::CXX)
                containsCXX = true;
        }

        for (auto& [dir, cfg] : m_Projects.ExternalProjects)
        {
            if (cfg.Language == LanguageType::C)
                containsC = true;
            if (cfg.Language == LanguageType::CXX)
                containsCXX = true;
        }
        std::stringstream lss;
        if (containsC)
            lss << " C";
        if (containsCXX)
            lss << " CXX";

        ss << "cmake_minimum_required(VERSION " << m_Version << ")\n"
           << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n"
           << "project(" << m_WorkspaceConfig.Name << " LANGUAGES" << lss.str() << ")\n"
           << "set(GDEFINES\n";
           for (auto& def : m_WorkspaceConfig.Defines)
               ss << "    " << def << std::endl;

        ss << ")\n\n";

        for (auto& cfg :m_Projects.InlineProjects)
            ss << BuildProject(cfg);

        for (auto& [dir, cfg] : m_Projects.ExternalProjects)
        {
            ss << "add_subdirectory(" << dir << ")\n";

            std::stringstream ess;
            ess << BuildProject(cfg);

            std::stringstream dirStr;
            dirStr << dir << "/CMakeLists.txt";

            std::ofstream out(dirStr.str());
            if (!out.is_open())
            {
                std::stringstream err;
                err << "Could not generate - " << dirStr.str();
                Utils::PrintWarning(err);
            }
            out << ess.str();
            out.close();
        }

        std::ofstream out("CMakeLists.txt");
        if (!out.is_open())
        {
            std::stringstream err;
            Utils::PrintWarning("Could not generate root CMakeLists.txt");
            return;
        }
        out << ss.str();
        out.close();

    }

    std::string CMakePlatform::BuildProject(ProjectConfig& cfg)
    {
        std::stringstream ss;

        ss << "set(SRCS\n";

        for (auto& src : cfg.Files)
        {
            ss << "    " << src <<  std::endl;
        }

        ss << ")\n";

        switch (cfg.Kind)
        {
            case KindType::STATICLIB:
            {
                ss << "add_library(" << cfg.Name << " STATIC ${SRCS})\n";
                break;
            }
            case KindType::SHAREDLIB:
            {
                ss << "add_library(" << cfg.Name << " SHARED ${SRCS})\n";
                break;
            }
            case KindType::CONSOLEAPP:
            {
                ss << "add_executable(" << cfg.Name << " ${SRCS})\n";
                break;
            }
            default:
                break;
        }

        switch (cfg.Language)
        {
            case LanguageType::C:
            {
                ss << "set_property(TARGET " << cfg.Name << " PROPERTY C_STANDARD " << cfg.Dialect << ")\n";
                break;
            }
            case LanguageType::CXX:
            {
                ss << "set_property(TARGET " << cfg.Name << " PROPERTY CXX_STANDARD " << cfg.Dialect << ")\n";
                break;
            }
            default:
                break;
        }


        if (!cfg.IncludedDirectories.empty())
        {
            ss << "target_include_directories(" << cfg.Name << " PRIVATE\n";

            for (auto& src :cfg.IncludedDirectories)
            {
                ss << "    " << src <<  std::endl;
            }

            ss << ")\n";
        }

        if (!cfg.Links.empty())
        {
            ss << "target_link_libraries(" << cfg.Name << "\n";

            for (auto& src :cfg.Links)
            {
                ss << "    " << src <<  std::endl;
            }

            ss << ")\n";
        }
        return ss.str();
    }


    // UTILITY ------------------------------------------------------------------------------------

    CMakePlatform::ProjectType CMakePlatform::CheckProjectType(const std::string& line)
    {
        std::string lineStr = line;
        lineStr.erase(remove_if(lineStr.begin(), lineStr.end(), isspace), lineStr.end());
        const char* lineCSTR = lineStr.c_str();
        std::string typeStr;
        for (int i = 0; i < 7; i++)
        {
            std::stringstream ss;
            ss << lineCSTR[i];
            typeStr.append(ss.str());
        }
        if (typeStr == "project")
        {
            return ProjectType::INLINE;
        }
        if (typeStr == "externa")
        {
            return ProjectType::EXTERNAL;
        }
        return ProjectType::pNONE;
    }

    // UTILITY ----------------------------------------------------------------------------------------------------------------

    bool CMakePlatform::CheckSyntax(const std::string& strCache)
    {
        PB_PRINT("CheckSyntax");
        size_t pos = 0;
        while(pos != NPOS)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            if (line.find_first_of("\"") == NPOS)
            {
                std::string keyword = GetKeyword(line);

                if (!IsMultiParameter(keyword))
                {
                    std::stringstream ss;
                    ss << "Syntax Error - " << keyword;
                    Utils::PrintError(ss);
                }

                bool openBracketFound = false;

                if (line.find("{") != NPOS)
                    openBracketFound = true;

                while (true)
                {
                    pos = strCache.find_first_not_of("\r\n", eol);
                    eol = strCache.find_first_of("\r\n", pos);

                    std::string line = strCache.substr(pos, eol - pos);

                    if (line.find("{") != NPOS && !openBracketFound)
                    {
                        openBracketFound = true;
                        continue;
                    }
                    else if (line.find("{") == NPOS && !openBracketFound)
                    {
                        std::stringstream ss;
                        ss << "Syntax Error -'{' . Pos - " << pos;
                        Utils::PrintError(ss);
                        return false;
                    }

                    if (Utils::GetCharacterCount(line, '\"') != 2 && line.find("}") == NPOS)
                    {
                        std::stringstream ss;
                        ss << "Syntax Error - '\"' . Pos - " << pos;
                        Utils::PrintError(ss);
                        return false;
                    }

                    if (Utils::GetCharacterCount(line, ',') > 1 && line.find("}") == NPOS)
                    {
                        std::stringstream ss;
                        ss << "Syntax Error - ',' . Pos - " << pos;
                        Utils::PrintError(ss);
                        return false;
                    }
                    else if (Utils::GetCharacterCount(line, ',') < 1 && line.find("}") == NPOS)
                    {
                        size_t nextLinePos = strCache.find_first_not_of("\r\n", eol);
                        size_t nextEOL = strCache.find_first_of("\r\n", nextLinePos);

                        std::string nextLine = strCache.substr(nextLinePos, nextEOL - nextLinePos);

                        if (Utils::GetCharacterCount(nextLine, '}') != 1)
                        {
                            std::stringstream ss;
                            ss << "Syntax Error - ',' . Pos - " << pos;
                            Utils::PrintError(ss);
                            return false;
                        }

                    }

                    if (line.find("}") != NPOS)
                    {
                        break;
                    }
                }
            }
            else if (Utils::GetCharacterCount(line, '\"') != 2)
            {
                std::stringstream ss;
                ss << "Syntax Error - '\"' . Pos - " << pos;
                Utils::PrintError(ss);
                return false;
            }
            pos = strCache.find_first_not_of("\r\n", eol);
            if (pos == NPOS)
            {
                break;
            }
        }
        return true;
    }

    std::string CMakePlatform::GetKeyword(const std::string& line)
    {
        PB_PRINT("GetKeyword");
        for (auto& keyword : Keywords)
        {
            if (line.find(keyword) != NPOS)
            {
                return keyword;
            }
        }
        return line;
    }

    bool CMakePlatform::IsMultiParameter(const std::string& keyword)
    {
        PB_PRINT("IsMultiParameter");
        if (keyword == "configurations")
            return true;
        if (keyword == "defines")
            return true;
        if (keyword == "files")
            return true;
        if (keyword == "includedirs")
            return true;
        return false;
    }

    bool CMakePlatform::IsSetForMultipleParameters(const std::string& strCache, size_t& outPos)
    {
        size_t pos = outPos;
        for (int i = 0; i < 2; i++)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            if (line.find("{") != NPOS)
            {
                return true;
            }
            pos = strCache.find_first_not_of("\r\n", eol);
            if (pos == NPOS)
                return false;
        }
        return false;
    }

    bool CMakePlatform::ContainsKeyword(const std::string& line, std::string& outKeyword, bool isFilePath)
    {
        if (isFilePath)
        {
            for (auto& keyword : PathKeywords)
            {
                if (line.find(keyword) != NPOS)
                {
                    outKeyword = keyword;
                    return true;
                }
            }
        }
        else
        {
            for (auto& keyword : Keywords)
            {
                if (line.find(keyword) != NPOS)
                {
                    outKeyword = keyword;
                    return true;
                }
            }
        }
        return false;
    }

    std::string CMakePlatform::ParseField(const std::string& line, const std::string& keyword)
    {
        std::string out = line;
        out.erase(remove_if(out.begin(), out.end(), isspace), out.end());
        out.erase(std::remove(out.begin(), out.end(), '"'), out.end());
        out.erase(0, keyword.length());

        return out;
    }

    std::vector<std::string> CMakePlatform::ParseMultipleFields(const std::string& strCache, size_t& outPos, const std::string& keyword)
    {
        std::vector<std::string> out;
        size_t pos = outPos;
        while (pos != NPOS)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            if (line.find(keyword) != NPOS)
            {
                pos = strCache.find_first_not_of("\r\n", eol);
                continue;
            }

            if (line.find("{") != NPOS)
            {
                pos = strCache.find_first_not_of("\r\n", eol);
                continue;
            }

            if (line.find("}") != NPOS)
            {
                break;
            }

            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            line.erase(std::remove(line.begin(), line.end(), ','), line.end());

            out.push_back(line);

            pos = strCache.find_first_not_of("\r\n", eol);
        }
        return out;
    }

    CMakePlatform::ArchitectureType CMakePlatform::StringToArchitectureType(const std::string line)
    {
        if (line == "x86")
            return ArchitectureType::X86;
        if (line == "x64")
            return ArchitectureType::X64;

        return ArchitectureType::aNONE;
    }

    CMakePlatform::LanguageType CMakePlatform::StringToLanguageType(std::string langStr)
    {
        if (langStr == "C")
            return LanguageType::C;
        if (langStr == "C++")
            return LanguageType::CXX;
        return LanguageType::lNONE;
    }
    CMakePlatform::KindType CMakePlatform::StringToKindType(std::string kindStr)
    {
        if (kindStr == "StaticLib")
            return KindType::STATICLIB;
        if (kindStr == "SharedLib")
            return KindType::SHAREDLIB;
        if (kindStr == "ConsoleApp")
            return KindType::CONSOLEAPP;
        return KindType::kNONE;
    }
}
