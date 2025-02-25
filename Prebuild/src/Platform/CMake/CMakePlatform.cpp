#include "CMakePlatform.h"
#include "Core/Utils.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

#define PB_PRINT(x) //printf("FUNCTION - %s\n", x)

namespace Prebuild
{

    CMakePlatform::CMakePlatform()
    {
        PB_PRINT("Constructor");
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


        while (pos != NPOS)
        {
            size_t eol = m_RootPrebuildString.find_first_of("\r\n", pos);
            std::string line = m_RootPrebuildString.substr(pos, eol - pos);
            ProjectType res = CheckProjectType(line);

            switch (res)
            {
                case ProjectType::INLINE:
                {
                    std::string projStr = ParseProject(pos);
                    if (projStr.empty())
                    {
                        return;
                    }
                    m_InlineProjectStrings.push_back(projStr);
                }
                case ProjectType::EXTERNAL:
                {
                    std::string dir = line;

                    dir.erase(remove_if(dir.begin(), dir.end(), isspace), dir.end());
                    dir.erase(std::remove(dir.begin(), dir.end(), '\"'), dir.end());
                    dir.erase(0, 8);

                    std::cout << dir << std::endl;

                    std::string projStr = ParseProject(pos, dir);
                    if (projStr.empty())
                    {
                        return;
                    }
                    m_ExternalProjectStrings.emplace(dir, projStr);

                    if(!CheckSyntax(m_ExternalProjectStrings.at(dir)))
                    {
                        return;
                    }


                }
                default:
                {
                    pos = m_RootPrebuildString.find_first_not_of("\r\n", eol);
                }
            }

        }

    }

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
        for (auto& keyword :Keywords)
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

}
