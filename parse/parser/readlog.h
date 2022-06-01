#ifndef SHADEWATCHER_PARSER_READLOG_H_
#define SHADEWATCHER_PARSER_READLOG_H_

#include <iostream>
#include <string>
#include <cinttypes>
#include <fstream>
#include <limits>
#include <memory>
#include <algorithm>
#include "common.h"
#include "util/normalize.h"

class LogLoader {
public:
	std::fstream *f_input;
	std::string path;
	std::vector<std::pair<event_t, uint64_t>> offset;
	event_t event_num;

	LogLoader(std::string LogPath);
	~LogLoader();

	bool GetEvent(event_t id, Json::Value &event);
	void LoadBEATLog();
    void LoadDARPALog();

    static void PrintBEATEvent(const Json::Value event);
};

#endif
