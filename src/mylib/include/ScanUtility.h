#pragma once

#include <iostream>
#include <string>
#include <map>
#include <filesystem>
#include <openssl/md5.h>
#include <vector>

#include "ParserCommandLine.h"

#define BUF_SIZE 4096

class ScanUtility
{
public:
    ScanUtility(const ParserCommandLine &parser);
    ~ScanUtility();
    void StartScan();

private:

    ParserCommandLine m_parserCommandLine;

    std::map<std::string, std::vector<unsigned char>> m_fileToMD5;
    std::vector<std::vector<std::string> > m_dataCsv;

    void ScanFolder(const std::string& path);
    int getMd5HashFile(const std::string& path, std::vector<unsigned char>& hash);

};
