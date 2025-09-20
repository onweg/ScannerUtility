#pragma once

#include <string>

#include "CLI11.hpp"

class ParserCommandLine {
public:
    ParserCommandLine();

    int startParse(int argc, char **argv);

    std::string getPathToBase();
    std::string getPathToLog();
    std::string getPpathToStartDir();

private:
    std::string m_pathToBase = "";
    std::string m_pathToLog = "";
    std::string m_pathToStartDir = "";
};