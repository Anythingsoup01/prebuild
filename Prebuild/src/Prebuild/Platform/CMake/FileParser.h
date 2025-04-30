#pragma once

// The purpose of this file is to loop through all directories 
// after the root file directory to find all prebuild files
// This is to be able to parse everything no matter what.
// If we call the "external" command it will lookup from a list
// of these files and pop the information in.

struct FileData
{
    std::string FileString;
    bool IsRoot;
};

class FileParser
{
public:
    void ParseAllPrebuildFiles();


    std::unordered_map<std::string, FileData>& GetFiles() const { return m_Files; }
private:
    std::unordered_map<std::string, FileData> m_Files;
};
