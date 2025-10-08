#include "pbpch.h"
#include "Platform.h"
#include "Platform/CMake/CMakePlatform.h"


void stackdump(lua_State* L) {
    int i;
    int top = lua_gettop(L);
    std::cout << "--- Stack Dump (Top: " << top << ") ---" << std::endl;
    for (i = 1; i <= top; i++) {
        int t = lua_type(L, i);
        std::cout << i << ": ";
        switch (t) {
            case LUA_TSTRING:
                std::cout << "string: '" << lua_tostring(L, i) << "'";
                break;
            case LUA_TBOOLEAN:
                std::cout << "boolean: " << (lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                std::cout << "number: " << lua_tonumber(L, i);
                break;
            case LUA_TTABLE:
                std::cout << "table: (addr:" << lua_topointer(L, i) << ")";
                break;
            case LUA_TNIL:
                std::cout << "nil";
                break;
            default:
                std::cout << lua_typename(L, t);
                break;
        }
        std::cout << std::endl;
    }
    std::cout << "--- End Stack Dump ---" << std::endl;
}


namespace Prebuild
{
    static const char* PathKeywords[1] =
    {
        "${WORKSPACEDIR}",
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

    bool Platform::StrEqual(const std::string& in, const std::string& check)
    {
        if (strncmp(in.c_str(), check.c_str(), check.length()) == 0)
            return true;
        return false;
    }

    std::vector<std::string> GetTableVariables(lua_State* L, const std::string variableName)
    {
        std::vector<std::string> out;
        if (!lua_istable(L, -1) && lua_isstring(L, -1))
        {
            out.push_back(lua_tostring(L, -1));
        }
        else
        {
            lua_pushnil(L);
            int index = lua_gettop(L) - 1;

            while(lua_next(L, index) != 0)
            {
                if (lua_isstring(L, -1))
                {
                    out.push_back(lua_tostring(L, -1));
                }
                lua_pop(L, 1);
            }
        }
        return out;
    }

    std::string GetStringVariable(lua_State* L, const std::string& variableName)
    {
        std::vector<std::string> out = GetTableVariables(L, variableName);
        return out[0];
    }

    Platform::Platform(const Utils::System& system, const std::filesystem::path& searchDirectory)
        : m_System(system), m_SearchDirectory(searchDirectory)
    {
        lua_State* state = luaL_newstate();
        if (!state)
        {
            std::cerr << "Failed to create lua state/\n";
            return;
        }

        luaL_openlibs(state);

        std::filesystem::path filePath = m_SearchDirectory / "prebuild.lua";

        std::string formattedFile = FormatFileToLua(filePath);
        if (formattedFile.empty())
            return;


        if (luaL_dostring(state, formattedFile.c_str()) != LUA_OK)
        {
            std::cerr << "Failed to execute lua script: " << lua_tostring(state, -1) << std::endl;
            lua_close(state);
            return;
        }
        m_WorkspaceConfig = GetWorkspaceVariables(state);
        m_WorkspaceConfig.WorkingDirectory = m_SearchDirectory;


        for (int i = 0; i < m_CurrentProjectCount; i++)
        {
            std::string varName = "Project" + std::to_string(i);
            ProjectConfig cfg = GetInlineProject(state, varName);
            cfg.External = false;
            m_Projects.push_back(cfg);
        }


        for (int i = 0; i < m_CurrentExternalCount; i++)
        {
            std::string extName = "External";
            extName.append(std::to_string(i));
            lua_getglobal(state, extName.c_str());


            std::string variable = GetStringVariable(state, extName);
            m_WorkspaceConfig.Externals.push_back(variable);

            std::filesystem::path relativePath = m_SearchDirectory / variable;
            m_TMPPaths.push_back(relativePath);
            lua_pop(state, 1);
        }

        m_CurrentExternalCount = 0;
        m_CurrentProjectCount = 0;


        while (!m_TMPPaths.empty())
        {
            m_ExternalPaths = m_TMPPaths;
            m_TMPPaths.clear();

            for (auto& path : m_ExternalPaths)
            {
                m_Projects.push_back(GetExternalProject(path));
            }
        }

        auto platform = Create(m_WorkspaceConfig, m_Projects);

    }

    std::string Platform::FormatFileToLua(const std::filesystem::path& filePath)
    {
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            std::cerr << "Failed to open: " << filePath.generic_string() << std::endl;
            return std::string();
        }
        std::stringstream ss;
        ss << in.rdbuf();
        in.close();

        std::stringstream out;

        std::string line;
        while (getline(ss, line))
        {
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            std::string parsedLine = line;
            if (StrEqual(line, "Project"))
            {
                parsedLine = "Project";
                parsedLine.append(std::to_string(m_CurrentProjectCount++));

                std::string tmp = line.substr(strlen("Project"));
                parsedLine.append(tmp);
            }
            else if (StrEqual(line, "External"))
            {
                parsedLine = "External";
                parsedLine.append(std::to_string(m_CurrentExternalCount++));

                std::string tmp = line.substr(strlen("External"));
                parsedLine.append(tmp);
            }

            out << parsedLine << std::endl;
        }

        return out.str();
    }


    Platform::WorkspaceConfig Platform::GetWorkspaceVariables(lua_State* L)
    {
        WorkspaceConfig cfg;
        lua_getglobal(L, "Workspace");


        if (!lua_istable(L, -1))
        {
            std::cerr << "Workspace must be a table!\n";
            return cfg;
        }

        lua_pushnil(L);
        int tableIndex = lua_gettop(L) - 1;

        while (lua_next(L, tableIndex) != 0)
        {
            std::string key = lua_tostring(L, -2);
            if (StrEqual(key, "name"))
            {
                cfg.Name = GetStringVariable(L, "name");
            }
            else if (StrEqual(key, "architecture"))
            {
                cfg.Architecture = StringToArchitectureType(GetStringVariable(L, "architecture"));
            }
            else if (StrEqual(key, "configurations"))
            {
                cfg.Configurations = GetTableVariables(L, "configurations");
            }
            else if (StrEqual(key, "defines"))
            {
                cfg.Defines = GetTableVariables(L, "defines");
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);


        if (cfg.Configurations.empty())
        {
            cfg.Configurations = {"Debug", "Release"};
        }
        return cfg;
    }

    Platform::ProjectConfig Platform::GetInlineProject(lua_State* L, const std::string& varName)
    {
        ProjectConfig cfg = GetProjectVariables(L, varName, ".");
        cfg.External = false;
        return cfg;
    }

    Platform::ProjectConfig Platform::GetExternalProject(const std::filesystem::path& path)
    {
        lua_State* state = luaL_newstate();
        if (!state)
        {
            std::cerr << "Failed to create lua state/\n";
            return {};
        }

        luaL_openlibs(state);

        std::filesystem::path filePath = path / "prebuild.lua";

        std::string formattedFile = FormatFileToLua(filePath);
        if (formattedFile.empty())
            return {};

        if (luaL_dostring(state, formattedFile.c_str()) != LUA_OK)
        {
            std::cerr << "Failed to execute lua script: " << lua_tostring(state, -1) << std::endl;
            lua_close(state);
            return {};
        }

        ProjectConfig cfg;
        if (m_CurrentProjectCount != 0)
            cfg = GetProjectVariables(state, "Project0", m_SearchDirectory / path);
        cfg.External = true;

        for (int i = 0; i < m_CurrentExternalCount; i++)
        {
            std::string extName = "External";
            extName.append(std::to_string(i));
            lua_getglobal(state, extName.c_str());


            std::string variable = GetStringVariable(state, extName);
            cfg.Externals.push_back(variable);

            std::filesystem::path relativePath = m_SearchDirectory / path / variable;
            m_TMPPaths.push_back(relativePath);

            lua_pop(state, 1);
        }

        m_CurrentExternalCount = 0;
        m_CurrentProjectCount = 0;

        lua_close(state);

        cfg.Directory = path;

        return cfg;
    }


    Platform::ProjectConfig Platform::GetProjectVariables(lua_State* L, const std::string& varName, const std::filesystem::path& path)
    {
        ProjectConfig cfg;
        lua_getglobal(L, varName.c_str());


        if (!lua_istable(L, -1))
        {
            std::cerr << "Project must be a table!\n";
            return cfg;
        }

        lua_pushnil(L);
        int tableIndex = lua_gettop(L) - 1;

        while (lua_next(L, tableIndex) != 0)
        {
            std::string key = lua_tostring(L, -2);
            if (StrEqual(key, "name"))
            {
                cfg.Name = GetStringVariable(L, "name");
            }
            else if (StrEqual(key, "language"))
            {
                cfg.Language = StringToLanguageType(GetStringVariable(L, "language"));
            }
            else if (StrEqual(key, "dialect"))
            {
                cfg.Dialect = GetStringVariable(L, "dialect");
            }
            else if (StrEqual(key, "kind"))
            {
                cfg.Kind = StringToKindType(GetStringVariable(L, "kind"));
            }
            else if (StrEqual(key, "pch"))
            {
                cfg.PrecompiledHeader = GetStringVariable(L, "pch");
            }
            else if (StrEqual(key, "files"))
            {
                std::vector<std::string> tmp = GetTableVariables(L, "files");
                for (auto file : tmp)
                {
                    cfg.Files.push_back(file);
                }
            }
            else if (StrEqual(key, "includedirs"))
            {
                std::vector<std::string> tmp = GetTableVariables(L, "includedirs");
                for (auto file : tmp)
                {
                    cfg.IncludedDirectories.push_back(file);
                }
            }
            else if (StrEqual(key, "links"))
            {
                cfg.Links = GetTableVariables(L, "links");
            }
            else if (StrEqual(key, "defines"))
            {
                cfg.Defines = GetTableVariables(L, "links");
            }
            else if (StrEqual(key, "filters"))
            {
                if (lua_istable(L, -1))
                {
                    lua_pushnil(L);
                    int configIndex = lua_gettop(L) - 1;

                    FilterConfig fcfg;

                    while(lua_next(L, configIndex) != 0)
                    {
                        cfg.Filters.push_back(ProcessFilter(L));
                        lua_pop(L, 1);
                    }
                }
            }

            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        return cfg;

    }

    Platform::FilterConfig Platform::ProcessFilter(lua_State* L)
    {
        FilterConfig fcfg;
        if (lua_istable(L, -1))
        {
            lua_pushnil(L);
            int configIndex = lua_gettop(L) - 1;


            while(lua_next(L, configIndex) != 0)
            {
                std::string key = lua_tostring(L, -2);

                if (StrEqual(key, "name"))
                {
                    fcfg.Name = GetStringVariable(L, "name");
                }
                else if (StrEqual(key, "files"))
                {
                    std::vector<std::string> tmp = GetTableVariables(L, "files");
                    for (auto& file : tmp)
                        fcfg.Files.push_back(file);
                }
                else if (StrEqual(key, "defines"))
                {
                    fcfg.Defines = GetTableVariables(L, "defines");
                }
                else if (StrEqual(key, "links"))
                {
                    fcfg.Links = GetTableVariables(L, "links");
                }

                lua_pop(L, 1);
            }
        }
        return fcfg;
    }

    Platform::ArchitectureType Platform::StringToArchitectureType(const std::string& archStr)
    {
        if (StrEqual(archStr, "x86"))
            return ArchitectureType::X86;
        if (StrEqual(archStr, "x64"))
            return ArchitectureType::X64;

        return ArchitectureType::NONE;

    }

    Platform::LanguageType Platform::StringToLanguageType(const std::string& langStr)
    {
        if (langStr == "C")
            return LanguageType::C;
        if (langStr == "C++")
            return LanguageType::CXX;
        return LanguageType::NONE;
    }

    Platform::KindType Platform::StringToKindType(const std::string& kindStr)
    {
        if (kindStr == "StaticLib")
            return KindType::STATICLIB;
        if (kindStr == "SharedLib")
            return KindType::SHAREDLIB;
        if (kindStr == "ConsoleApp")
            return KindType::CONSOLEAPP;
        return KindType::NONE;
    }

    Scope<Platform> Platform::Create(const WorkspaceConfig& workspaceConfig, const std::vector<ProjectConfig>& projectConfigs)
    {
        switch (m_System)
        {
            case Utils::System::CMAKE: return CreateScope<CMakePlatform>(workspaceConfig, projectConfigs);
            default: return nullptr;
        }
    }
}
