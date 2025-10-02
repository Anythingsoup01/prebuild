#pragma once
#include "Core.h"
#include "Core/Utils.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

const size_t NPOS = std::string::npos;

namespace Prebuild
{
    class Platform
    {
    private:
        enum class ProjectType
        {
            NONE = 0,
            INLINE,
            EXTERNAL,
        };

        enum class KindType
        {
            NONE = 0,
            STATICLIB,
            SHAREDLIB,
            CONSOLEAPP,
        };

        enum class LanguageType
        {
            NONE = 0,
            C,
            CXX,
        };

        enum class ArchitectureType
        {
            NONE = 0,
            X86,
            X64,
        };

        enum class KeywordType
        {
            NONE = 0,
            WORKSPACE,
            PROJECT,
            FILTER,
            FILEPATH,
        };
    public:
        struct WorkspaceConfig
        {
            std::string Name;
            ArchitectureType Architecture;
            std::filesystem::path FilePath;

            std::vector<std::string> Configurations;
            std::vector<std::string> Defines;
        };

        struct FilterConfig
        {
            std::string FilterParameter;

            std::vector<std::string> Defines;
            std::vector<std::string> Links;
        };

        struct ProjectConfig
        {
            std::string Directory;
            std::string Name;
            std::string Dialect;

            LanguageType Language;
            KindType Kind;

            std::string PrecompiledHeader;

            bool External;

            std::vector<std::string> Files;
            std::vector<std::string> IncludedDirectories;
            std::vector<std::string> Links;
            std::vector<std::string> Defines;

            std::vector<FilterConfig> Filters;
        };

        WorkspaceConfig m_WorkspaceConfig;
        std::vector<ProjectConfig> m_Projects;

    public:
        Platform(const std::filesystem::path& searchDirectory = std::filesystem::current_path());
        ~Platform() {}

    private:

        WorkspaceConfig ParseWorkspace(const std::string& in, std::stringstream& out);
        ProjectConfig ParseProject(const std::string& in, std::stringstream& out, bool isExternal, const std::filesystem::path& parentPath = "");
        ProjectConfig ParseExternalProject(const std::filesystem::path& path);


        WorkspaceConfig GenerateWorkspaceConfig(std::stringstream& ss);
        ProjectConfig GenerateProjectConfig(const std::string& strCache, bool isExternal, const std::string& path, const std::string& originalPath);
        FilterConfig GenerateFilterConfig(const std::string& strCache, size_t* outPos, const std::string& keyword, const std::string& projectName, bool isExternal);
        
        std::string ParseField(const std::string& line, const std::string& keyword);
        std::vector<std::string> ParseMultipleFields(const std::string& strCache, size_t& outPos, const std::string& keyword);
        std::vector<std::string> GetMultipleFields(const std::string& strCache, size_t& outPos, const std::string& keyword);

        std::vector<std::string> GetAllFilesWithExtension(const std::string& line, const std::string& extension, const std::string folderName = "");
        std::vector<std::string> SearchDirectoryFor(const std::filesystem::path filePath, const std::string extension);

        ArchitectureType StringToArchitectureType(const std::string archStr);
        LanguageType StringToLanguageType(std::string langStr);
        KindType StringToKindType(std::string kindStr);

        ProjectType CheckProjectType(const std::string& line);

        void Create();


        bool CheckSyntax(const std::string& in);
        std::string GetKeyword(const std::string& line);
        bool IsMultiParameter(const std::string& keyword);
        bool IsSetForMultipleParameters(const std::string& strCache, size_t& outPos);
        bool ContainsKeyword(const std::string& line, std::string& outKeyword, const KeywordType type);
    private:
        std::filesystem::path m_SearchDirectory;

        std::vector<std::filesystem::path> m_ExternalPaths;
        std::vector<std::filesystem::path> m_TMPPaths;
    };
}
