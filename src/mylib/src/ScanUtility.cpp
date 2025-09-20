#include "../include/ScanUtility.h"

ScanUtility::ScanUtility(const ParserCommandLine &parser)
:m_parserCommandLine(parser)
{
}

ScanUtility::~ScanUtility()
{
}

void ScanUtility::StartScan() {
    ScanFolder(m_parserCommandLine.getPpathToStartDir());
    for (const auto[key, value] : m_fileToMD5) {
        std::cout << key << "\t";
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
                    m_fileToMD5[entry.path().string()] = hashTmp;
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