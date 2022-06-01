#ifndef SHADEWATCHER_UTIL_CMDARGUMENT_H_
#define SHADEWATCHER_UTIL_CMDARGUMENT_H_

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

class CmdArgument 
{
public:
    std::vector <std::string> tokens;

    CmdArgument(int, char **);

    std::string GetComOption(std::string);
    std::string GetSecComOption(std::string);
    std::string GetNextComOption(std::string);
    bool ComOptionExist(std::string);
    void ComDarHelp();
    void ComBeatHelp();
    void ComKafkaHelp();
};

#endif
