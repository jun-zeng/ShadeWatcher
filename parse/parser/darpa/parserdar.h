#ifndef SHADEWATCHER_PARSER_DARPA_PARSERDAR_H_
#define SHADEWATCHER_PARSER_DARPA_PARSERDAR_H_

#include "parser/readlog.h"
#include "parser/kg.h"
#include "tripletdar.h"
#include "multithread.h"
#include "util/config.h"
#include "util/reduce_noise.h"
#include "util/file.h"
#include <pthread.h>

int BatchSize(event_t);
void KGConstruction(std::string, KG *, Config&);
void KGDarpaParse(std::string, KG *, int);

#endif
