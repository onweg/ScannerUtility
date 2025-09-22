#include "../include/CsvReader.h"

CsvReader::CsvReader()
{
}

int CsvReader::readCsv(const std::string &path)
{
    m_data.clear();
    std::fstream file(path);
    if (!file.is_open()) {
        std::cout << "Error opening file to path: " << path;
        return 1;
    }
    std::string line;
    int row = 0;
    while (getline(file, line)) {
        m_data.push_back({});
        std::stringstream ss(line);
        std::string cell;
        while (getline(ss, cell, ',')) {
            m_data[row].push_back(cell);
        }
        row++;
    }
    file.close();
    return 0;
}

std::vector<std::vector<std::string> > CsvReader::getData()
{
    return m_data;
}