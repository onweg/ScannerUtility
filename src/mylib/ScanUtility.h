#pragma once

#include <iostream>
#include <string>
#include <map>
#include <filesystem>
#include <openssl/md5.h>
#include <fstream>
#include <vector>
#include "CLI11.hpp"

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
    void parseCommandLine(int argc, char** argv);
public:
    ScanUtility(int argc, char** argv);
    ~ScanUtility();
    void StartScan();
};
