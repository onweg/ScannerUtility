#pragma once

#include <iostream>
#include <string>
#include <map>
#include <filesystem>

#ifdef MYLIB_EXPORTS
    #define MYLIB __declspec(dllexport)
#else
    #define MYLIB __declspec(dllimport)
#endif

class MYLIB ScanUtility
{
private:
    std::string _basePath;
    std::string _reportLogPath;
    std::string _folderPath;

    std::map<std::string, int> _fileToMD5;

    void ScanFolder(const std::string& path);

public:
    ScanUtility(const std::string& basePath, const std::string& reportLogPath, const std::string& folderPath);
    ~ScanUtility();
    void StartScan();
};
