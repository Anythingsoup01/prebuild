#include "pbpch.h"
#include "Platform.h"

#include <signal.h>

#include <lua.hpp>



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


    struct Optional
    {
        int Value = 0;
        bool HasChanged = false;

        Optional operator++(int)
        {
            ++Value;
            HasChanged = true;
            return *this;
        }

        Optional operator--(int)
        {
            --Value;
            HasChanged = true;
            return *this;
        }


        operator bool()
        {
            return HasChanged;
        }
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
        /*
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

        ParseWorkspace(rootSS.str(), rootSS);

        std::string line;
        while (getline(rootSS, line))
        {
            ProjectType res = CheckProjectType(line);

            ProjectConfig cfg = ParseProject(rootSS.str(), rootSS, false, m_SearchDirectory);
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

        printf("Workspace - %s\n", m_WorkspaceConfig.Name.c_str());

        for (auto& define : m_WorkspaceConfig.Configurations)
        {
            printf("Config - %s\n", define.c_str());
        }

        for (auto& cfg : m_Projects)
        {
            printf("%s\n", cfg.Name.c_str());
        } */

        lua_State* state = luaL_newstate();
        if (!state)
        {
            std::cerr << "Failed to create lua state/\n";
            return;
        }

        luaL_openlibs(state);

        std::filesystem::path filePath = m_SearchDirectory / "prebuild.lua";
        if (luaL_dofile(state, filePath.generic_string().c_str()) != LUA_OK)
        {
            std::cerr << "Failed to execute lua script: " << lua_tostring(state, -1) << std::endl;
            lua_close(state);
            return;
        }
        //lua_getglobal(state, "workspace");
        //if (lua_isstring(state, -1))
        //{
        //    m_WorkspaceConfig.Name = lua_tostring(state, -1);
        //    std::cout << m_WorkspaceConfig.Name << std::endl;
        //}
    }

    void Platform::ParseWorkspace(const std::string& in, std::stringstream& outSS)
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
            else if (!line.empty())
            {
                out << line << std::endl;
            }
        }

        if (!projectFound)
        {
            std::cerr << "ERROR: No Projects Found!\n";
            return;
        }

        outSS.str(out.str());

        GenerateWorkspaceConfig(workspaceStr);
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


            if (!externalFound && !projectFound && !line.empty())
            {
                projectStr << line << std::endl;
            }
            else if (projectFound && !externalFound && !line.empty())
            {
                outSS << line << std::endl;
            }

            firstLoop = false;
        }

        out.str(outSS.str());


        return {}; //GenerateProjectConfig(projectStr.str(), isExternal, dir, m_SearchDirectory);
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
    
    void Platform::GenerateWorkspaceConfig(std::stringstream& ss)
    {
        WorkspaceConfig cfg;

        std::string line;
        while (getline(ss, line))
        {
            std::string keyword;
            if (ContainsKeyword(line, keyword, KeywordType::WORKSPACE))
            {
                if (StrEqual(keyword, "workspace"))
                    cfg.Name = ParseField(line, keyword);
                else if (StrEqual(keyword, "architecture"))
                    cfg.Architecture = StringToArchitectureType(ParseField(line, keyword));
                else if (StrEqual(keyword, "configurations"))
                   cfg.Configurations = ParseMultipleFields(ss.str(), ss, keyword);
                else if (StrEqual(keyword, "defines"))
                    cfg.Defines = ParseMultipleFields(ss.str(), ss, keyword);
            }
        }

        if (cfg.Configurations.empty())
        {
            cfg.Configurations.push_back("Debug");
            cfg.Configurations.push_back("Release");
        }

        cfg.FilePath = m_SearchDirectory;
        m_WorkspaceConfig = cfg;
    }
    Platform::ProjectConfig Platform::GenerateProjectConfig(const std::string& strCache, bool isExternal, const std::string& path, const std::string& originalPath)
    {

    }
    Platform::FilterConfig Platform::GenerateFilterConfig(const std::string& strCache, size_t* outPos, const std::string& keyword, const std::string& projectName, bool isExternal)
    {

    }

    std::string Platform::ParseField(const std::string& line, const std::string& keyword)
    {
        std::string out = line;
        out.erase(remove_if(out.begin(), out.end(), isspace), out.end());
        out.erase(std::remove(out.begin(), out.end(), '"'), out.end());
        out.erase(0, keyword.length());

        return out;

    }

    std::vector<std::string> Platform::ParseMultipleFields(const std::string& in, std::stringstream& out, const std::string& keyword)
    {
        std::vector<std::string> fields;
        std::stringstream outSS;
        std::stringstream ss; ss << in;

        if (IsSetForMultipleParameters(in, keyword))
        {
            std::string line;

            Optional brackets;

            bool keywordFound = false;

            while (getline(ss, line))
            {
                int quotes = 0;

                if (StrEqual(line, keyword))
                {
                    keywordFound = true;
                }

                if (!keywordFound)
                {
                    outSS << line << std::endl;
                    continue;
                }
                std::string field;

                std::vector<char> chars(line.data(), line.data() + line.length());

                for (auto& c : chars)
                {
                    if (quotes == 1)
                    {
                        if (c != '\"')
                            field += c;
                    }


                    if (c == '{')
                        brackets++;
                    else if (c == '}')
                        brackets--;
                    else if (c == '\"')
                        quotes++;

                    if (quotes == 2)
                    {
                        if (std::find(fields.begin(), fields.end(), field) == fields.end())
                        fields.push_back(field);
                    }
                }

                if (brackets.HasChanged && brackets.Value == 0)
                    keywordFound = false;



            }
        }
        else
        {
            std::string line;
            getline(ss, line);
            fields.push_back(ParseField(line, keyword));
        }
        return fields;

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

    Platform::ArchitectureType Platform::StringToArchitectureType(const std::string archStr)
    {
        if (StrEqual(archStr, "x86"))
            return ArchitectureType::X86;
        if (StrEqual(archStr, "x64"))
            return ArchitectureType::X64;

        return ArchitectureType::NONE;

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

    bool Platform::IsSetForMultipleParameters(const std::string& in, const std::string& keyword)
    {
        bool keywordFound = false;

        std::stringstream ss; ss << in;
        std::string line;
        while (getline(ss, line))
        {
            if (StrEqual(line, keyword))
            {
                keywordFound = true;
            }

            if (!keywordFound)
                continue;

            std::vector<char> chars(line.data(), line.data() + line.length());

            for (auto& c : chars)
            {
                if (c == '{')
                    return true;
            }

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
