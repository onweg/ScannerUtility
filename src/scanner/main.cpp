#include <iostream>
#include "ScanUtility.h"
#include "CsvReader.h"

int main(int argc, char** argv) {
    ParserCommandLine parser;
    CsvReader r;
    if (!(parser.startParse(argc, argv) == 0 && r.readCsv(parser.getPathToBase()) == 0)) {
        return 1;
    }
    ScanUtility a(parser);
    a.StartScan();
    return 0;
}
