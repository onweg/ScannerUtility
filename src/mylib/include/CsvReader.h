#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class CsvReader {
public:
    CsvReader();

    int readCsv(const std::string &path);

    std::vector<std::vector<std::string> > getData();

private:
    std::vector<std::vector<std::string> > m_data;
};