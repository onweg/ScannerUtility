#pragma once

#include <iostream>
#include <string>
#include <map>
#include <filesystem>
#include <openssl/md5.h>
#include <fstream>
#include <vector>

#define BUF_SIZE 4096

class ScanUtility
{
private:
    std::string m_pathToBase;
    std::string m_pathToLog;
    std::string m_pathToStartDir;

    std::map<std::string, std::vector<unsigned char>> _fileToMD5;

    void ScanFolder(const std::string& path);
    int getMd5HashFile(const std::string& path, std::vector<unsigned char>& hash);
    void parseCommandLine(const int argc, const char** argv);
public:
    ScanUtility(const int argc, const char** argv);
    ~ScanUtility();
    void StartScan();
};
