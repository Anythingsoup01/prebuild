#include "pbpch.h"
#include "Platform.h"

#include <csignal>
#include <signal.h>
#include <sstream>

namespace Prebuild
{
    static const char* WorkspaceKeywords[4] =
    {
        "workspace",
        "architecture",
        "configurations",
        "defines",
    };

    static const char* ProjectKeywords[10]
    {
        "project",
        "language",
        "dialect",
        "kind",
        "pch",
        "files",
        "includedirs",
        "links",
        "filter",
        "defines"
    };

    static const char* FilterKeywords[3]
    {
        "filter",
        "defines",
        "links",
    };

    static const char* AllKeywords[13]
    {
        "workspace",
        "architecture",
        "configurations",
        "defines",
        "project",
        "language",
        "dialect",
        "kind",
        "pch",
        "files",
        "includedirs",
        "links",
        "filter",
        };

    static const char* PathKeywords[1] =
    {
        "$(WORKSPACEDIR)",
    };

    static bool StrEqual(const std::string& in, const std::string& check)
    {
        if (strncmp(in.c_str(), check.c_str(), check.length()) == 0)
            return true;
        return false;
    }


    Platform::Platform(const std::filesystem::path& searchDirectory)
    {
        m_SearchDirectory = searchDirectory;
        std::ifstream rootFile(searchDirectory / "prebuild.lua");
        if(!rootFile.is_open())
        {
            std::cerr << "Error: No prebuild.lua found!\n";
            return;
        }
        std::stringstream rootSS; rootSS << rootFile.rdbuf();
        rootFile.close();


        if (!CheckSyntax(rootSS.str()))
        {
            return;
        }
        m_WorkspaceConfig = ParseWorkspace(rootSS.str(), rootSS);
        std::string line;
        while (getline(rootSS, line))
        {
            ProjectType res = CheckProjectType(line);

            ProjectConfig cfg = ParseProject(rootSS.str(), rootSS);
            if (!cfg.Name.empty())
                m_Projects.push_back(cfg);

        }
        while (!m_TMPPaths.empty())
        {
            m_ExternalPaths = m_TMPPaths;
            m_TMPPaths.clear();

            for (auto& dir : m_ExternalPaths)
            {
                ProjectConfig cfg = ParseExternalProject(dir);
            }
        }
    }

    Platform::WorkspaceConfig Platform::ParseWorkspace(const std::string& in, std::stringstream& outSS)
    {
        std::stringstream workspaceStr;
        std::stringstream out;
        std::string line;

        bool projectFound = false;

        std::stringstream ss; ss << in;

        while(getline(ss, line))
        {
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            if (strncmp(line.c_str(), "project", strlen("project")) == 0 || strncmp(line.c_str(), "external", strlen("external")) == 0)
            {
                projectFound = true;
            }

            if (!projectFound)
            {
                workspaceStr << line << std::endl;
            }
            else
            {
                out << line << std::endl;
            }
        }

        if (!projectFound)
        {
            std::cerr << "ERROR: No Projects Found!\n";
            return WorkspaceConfig();
        }

        outSS.str(out.str());

        return GenerateWorkspaceConfig(workspaceStr);
    }

    Platform::ProjectConfig Platform::ParseProject(const std::string& in, std::stringstream& out, bool isExternal, const std::filesystem::path& parentPath)
    {
        std::stringstream projectStr;
        std::stringstream outSS;
        std::string dir;
        std::string line;

        bool projectFound = false;

        std::stringstream ss; ss << in;

        bool firstLoop = true;

        while(getline(ss, line))
        {
            bool externalFound = false;
            printf("%s\n", line.c_str());
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            if (StrEqual(line, "project") && !firstLoop && !projectFound)
            {
                projectFound = true;
            }
            else if (StrEqual(line, "external"))
            {
                std::string dir = line;
                dir.erase(remove_if(dir.begin(), dir.end(), isspace), dir.end());
                dir.erase(std::remove(dir.begin(), dir.end(), '\"'), dir.end());
                dir.erase(0, strlen("external"));


                m_TMPPaths.push_back(m_SearchDirectory / parentPath / dir);
                externalFound = true;
            }


            if (!externalFound && !projectFound)
            {
                projectStr << line << std::endl;
            }
            else if (projectFound && !externalFound)
            {
                outSS << line << std::endl;
            }

            firstLoop = false;
        }

        out.str(outSS.str());


        return GenerateProjectConfig(projectStr.str(), isExternal, dir, m_SearchDirectory);
    }

    Platform::ProjectConfig Platform::ParseExternalProject(const std::filesystem::path& path)
    {
        std::ifstream in(path / "prebuild.lua");
        if (!in.is_open())
        {
            std::cerr << "Failed to open '" << path.c_str() << "/prebuild.lua'!\n";
            return {};
        }

        std::stringstream inSS; inSS << in.rdbuf();

        return ParseProject(inSS.str(), inSS, true, path);
    }

    Platform::ProjectType Platform::CheckProjectType(const std::string& line)
    {
        if (strncmp(line.c_str(), "project", strlen("project")) == 0)
        {
            return ProjectType::INLINE;
        }
        if (strncmp(line.c_str(), "external", strlen("external")) == 0)
        {
            return ProjectType::EXTERNAL;
        }
        return ProjectType::NONE;
    }

    bool Platform::CheckSyntax(const std::string& in)
    {
        int quotes = 0; // These will simply be mod 2 to determine if we're missing any
        int brackets = 0; // We will add and subtract from this, if the number is not equal to 0, error
        std::string line;

        std::stringstream ss; ss << in;

        while (std::getline(ss, line))
        {
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());

            std::vector<char> characters(line.data(), line.data() + line.length());

            int inlineQuotes = 0;
            int inlineBrackets = 0;

            for (auto& c : characters)
            {
                if (c == '\"')
                {
                    quotes++;
                    inlineQuotes++;
                }
                else if (c == '{')
                {
                    brackets++;
                    inlineBrackets++;
                    }
                else if (c == '}')
                {
                    brackets--;
                    inlineBrackets--;
                }
            }

            std::string keyword(GetKeyword(line));

            if (!keyword.empty() && (brackets != 0 || quotes % 2 != 0))
            {
                std::cerr << "Syntax Error: Keyword before closing bracket / Too (many/little) quotes!";
                return false;
            }

            if (!IsMultiParameter(keyword) && !keyword.empty())
            {
                if (inlineQuotes <= 0)
                {
                    std::cerr << "Error at keyword \'" << keyword << "\' missing quotes!\n";
                    return false;
                }
            }
        }

        if (brackets != 0 || quotes % 2 != 0)
        {
            std::cerr << "Syntax Error: Missing closing bracket / quote!";
            return false;
        }
        return true;
    }

    std::string Platform::GetKeyword(const std::string& line)
    {
        for (auto& keyword : AllKeywords)
        {
            if (strncmp(line.c_str(), keyword, strlen(keyword)) == 0)
            {
                return keyword;
            }
        }
        return std::string();
    }

    bool Platform::IsMultiParameter(const std::string& keyword)
    {
        if (keyword == "configurations")
            return true;
        if (keyword == "defines")
            return true;
        if (keyword == "files")
            return true;
        if (keyword == "includedirs")
            return true;
        if (keyword == "links")
            return true;
        return false;
    }

    bool Platform::IsSetForMultipleParameters(const std::string& strCache, size_t& outPos)
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

    bool Platform::ContainsKeyword(const std::string& line, std::string& outKeyword, const KeywordType type)
    {
        switch (type)
        {
            case (KeywordType::WORKSPACE):
            {
                for (auto& keyword : WorkspaceKeywords)
                {
                    if (line.find(keyword) != NPOS)
                    {
                        outKeyword = keyword;
                        return true;
                    }
                }
                break;
            }

            case (KeywordType::PROJECT):
            {
                for (auto& keyword : ProjectKeywords)
                {
                    if (line.find(keyword) != NPOS)
                    {
                        outKeyword = keyword;
                        return true;
                    }
                }
                break;
            }

            case (KeywordType::FILTER):
            {
                for (auto& keyword : FilterKeywords)
                {
                    if (line.find(keyword) != NPOS)
                    {
                        outKeyword = keyword;
                        return true;
                    }
                }
                break;
            }

            case (KeywordType::FILEPATH):
            {
                for (auto& keyword : PathKeywords)
                {
                    if (line.find(keyword) != NPOS)
                    {
                        outKeyword = keyword;
                        return true;
                    }
                }
                break;
            }

            default: break;
        }

        return false;
    }

}
