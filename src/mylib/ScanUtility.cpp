#include "ScanUtility.h"

ScanUtility::ScanUtility(const int argc, const char** argv)
{
    parseCommandLine(argc, argv);
}

ScanUtility::~ScanUtility()
{
}

void ScanUtility::StartScan() {
    ScanFolder(m_pathToStartDir);
    for (const auto[key, value] : _fileToMD5) {
        std::cout << key << " ";
        for (const auto& c : value) {
            std::cout << c;
        }
        std::cout << "\n";
    }
}

void ScanUtility::ScanFolder(const std::string& path) {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            // if (entry.is_regular_file()) {
            //     std::cout << "skip file to path: " <<  entry.path().string() << "\n";
            // } else {
                std::vector<unsigned char> hashTmp;
                if (getMd5HashFile(entry.path().string(), hashTmp) == 0) {
                    _fileToMD5[entry.path().string()] = hashTmp;
                } else {
                    std::cout << "File undefined to path: " << entry.path().string() << "\n";
                }
            // }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Filesystem error: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "General error: " << e.what() << "\n";
    }
}

int ScanUtility::getMd5HashFile(const std::string& path, std::vector<unsigned char>& hash) {
    int result = 0;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        result = 1;
    } else {
        MD5_CTX md5;
        MD5_Init(&md5);
        char *buf = new char [BUF_SIZE];
        while (file.read(buf, BUF_SIZE) || file.gcount() > 0) {
            MD5_Update(&md5, buf, file.gcount());
        }
        hash.clear();
        hash.resize(MD5_DIGEST_LENGTH);
        MD5_Final(hash.data(), &md5);
    }
    return result;
}

void parseCommandLine(const int argc, const char** argv) {
    CLI::App app("Scanner");
    std::string pathToBase, pathToLog, pathToStartDir;
    app.add_option("--base", pathToBase, "Write your path to base")->required();
    app.add_option("--log", pathToLog, "Write your path to logfile")->required();
    app.add_option("--path", pathToStartDir, "Write your path to start dir")->required();

    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        std::cout << "Failed parse command line: " << app.exit(e);
    }
    m_pathToBase = pathToBase;
    m_pathToLog = pathToLog;
    m_pathToStartDir = pathToStartDir;
}