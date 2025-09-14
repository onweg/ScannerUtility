#include "ScanUtility.h"

ScanUtility::ScanUtility(const std::string &basePath, const std::string &reportLogPath, const std::string &folderPath)
:_basePath(basePath),_reportLogPath(reportLogPath), _folderPath(folderPath)
{
}

ScanUtility::~ScanUtility()
{
}

void ScanUtility::StartScan() {
    ScanFolder(_folderPath);
    for (const auto[key, value] : _fileToMD5) {
        std::cout << key << " " << value << "\n";
    }
}

void ScanUtility::ScanFolder(const std::string& path) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        _fileToMD5[entry.path().string()] = 1;
    }
}