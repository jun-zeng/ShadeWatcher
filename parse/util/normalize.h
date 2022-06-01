#ifndef SHADEWATCHER_UTIL_NORMALRIZE_H_
#define SHADEWATCHER_UTIL_NORMALRIZE_H_

#include <iostream>
#include <sstream> 
#include <string>
#include <stdexcept>
#include <memory>
#include "parser/common.h"

// Ignore warning due to old API in jsonapp
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

#if defined(__APPLE__)
	#include <json/json.h>
#else
	#include <jsoncpp/json/json.h>
#endif

std::string Jval2str(const Json::Value jval);
std::string Jval2strid(const Json::Value jval);
std::string Jval2strArgs(const Json::Value jval);
biguint_t Jval2int(const Json::Value jval);
biguint_t stoint128_t(std::string const & in);

#endif
