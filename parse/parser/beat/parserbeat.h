#ifndef SHADEWATCHER_PARSER_BEAT_PARSERBEAT_H_
#define SHADEWATCHER_PARSER_BEAT_PARSERBEAT_H_

#include "parser/readlog.h"
#include "parser/kg.h"
#include "tripletbeat.h"
#include "util/reduce_noise.h"

void Loadmetainfo(std::string, KG *);
void KGBeatParse(std::string , KG *);

#endif
