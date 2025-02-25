#include "CMakePlatform.h"
#include "Core/Utils.h"
#include <sstream>
#include <iostream>

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

    // UTILITY ----------------------------------------------------------------------------------------------------------------

    bool CMakePlatform::CheckSyntax(const std::string& strCache)
    {
        PB_PRINT("CheckSyntax");
        size_t pos = 0;
        int line = 0;
        while(pos != NPOS)
        {
            size_t eol = strCache.find_first_of("\r\n", pos);
            std::string line = strCache.substr(pos, eol - pos);

            line += 1;

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
                        ss << "Syntax Error -'{' . Line - " << line << ", Pos " << pos - eol;
                        Utils::PrintError(ss);
                        return false;
                    }

                    if (Utils::GetCharacterCount(line, '\"') != 2 && line.find("}") == NPOS)
                    {
                        std::cout << Utils::GetCharacterCount(line, '\"') <<std::endl;
                        std::stringstream ss;
                        ss << "Syntax Error - '\"' . Line - " << line << ", Pos " << pos - eol;
                        Utils::PrintError(ss);
                        return false;
                    }

                    if (Utils::GetCharacterCount(line, ',') > 1 && line.find("}") == NPOS)
                    {
                        std::stringstream ss;
                        ss << "Syntax Error - ',' . Line - " << line << ", Pos " << pos - eol;
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
                            ss << "Syntax Error - ',' . Line - " << line << ", Pos " << pos - eol;
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
                ss << "Syntax Error - '\"' . Line - " << line << ", Pos " << pos - eol;
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
