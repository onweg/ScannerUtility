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
    std::string _basePath;
    std::string _reportLogPath;
    std::string _folderPath;

    std::map<std::string, std::vector<unsigned char>> _fileToMD5;

    void ScanFolder(const std::string& path);
    int getMd5HashFile(const std::string& path, std::vector<unsigned char>& hash);
public:
    ScanUtility(const std::string& basePath, const std::string& reportLogPath, const std::string& folderPath);
    ~ScanUtility();
    void StartScan();
};
