#pragma once

#include <string>
#include <vector>

class CsvReader {
public:
    CsvReader();

    int readCsv(const std::string &path);

    std::vector<std::vector<std::string> > getData();

private:
     std::vector<std::vector<std::string> > data;
};