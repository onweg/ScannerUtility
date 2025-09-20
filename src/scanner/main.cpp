#include <iostream>
#include "ScanUtility.h"


void parseCommandLine(int argc, char** argv);
int readCsvFiles(std::vector<std::vector<std::string> > &data);

int main(int argc, char** argv) {

    ParserCommandLine parser;
    if (parser.startParse(argc, argv) == 0) {
        ScanUtility a(parser);
        a.StartScan();
    }
    return 0;
}
