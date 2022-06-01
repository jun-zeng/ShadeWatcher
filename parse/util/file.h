#ifndef SHADEWATCHER_UTIL_FILE_H_
#define SHADEWATCHER_UTIL_FILE_H_

#include <vector>
#include <string>
#include <dirent.h>
#include <iostream>
#include <algorithm>

std::vector<std::string> TraverseDir(std::string);
std::vector<std::string> TraverseFile(std::string);
std::vector<std::string> TraverseJsonFile(std::string);
std::vector<std::string> CollectJsonFile(std::string);
std::vector<std::string> CollectBeatFile(std::string);
std::vector<std::string> TraverseBeatDir(std::string);

#endif
