#ifndef SHADEWATCHER_KAFKA_PARSERKAFKA_H_
#define SHADEWATCHER_KAFKA_PARSERKAFKA_H_

#include "consumer.h"
#include "parser/kg.h"
#include "parser/beat/tripletbeat.h"
#include "util/reduce_noise.h"
#include "db/postgresql.h"

void KGKafkaParse(KG *, Config &);

#endif
