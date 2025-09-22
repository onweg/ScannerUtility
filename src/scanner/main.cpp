#include <iostream>
#include "ScanUtility.h"
#include "CsvReader.h"

int main(int argc, char** argv) {
    ParserCommandLine parser;
    CsvReader r;
    if (!(parser.startParse(argc, argv) == 0 && r.readCsv(parser.getPathToBase()) == 0)) {
        return 1;
    }
    std::vector<std::vector<std::string>> r_tmp = r.getData();
    for (auto i : r_tmp) {
        for (auto j : i) {
            std::cout << j << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n\n";
    ScanUtility a(parser);
    a.StartScan();
    return 0;
}
