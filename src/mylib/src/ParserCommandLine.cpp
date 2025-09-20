#include "../include/ParserCommandLine.h"

ParserCommandLine::ParserCommandLine()
{
}

int ParserCommandLine::startParse(int argc, char **argv)
{
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

    return 0;
}

std::string ParserCommandLine::getPathToBase()
{
    return m_pathToBase;
}

std::string ParserCommandLine::getPathToLog()
{
    return m_pathToLog;
}

std::string ParserCommandLine::getPpathToStartDir()
{
    return m_pathToStartDir;
}