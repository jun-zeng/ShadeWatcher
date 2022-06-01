#ifndef SHADEWATCHER_KAFKA_CONSUMER_H_
#define SHADEWATCHER_KAFKA_CONSUMER_H_

#include <ctime>

#include <iostream>
#include <getopt.h>
#include <csignal>
#include <map>
#include "librdkafka/rdkafkacpp.h"
#include "util/config.h"
#include "util/normalize.h"

typedef std::map<std::string, std::string> KafkaConfig;
std::map<seq_t, Json::Value> consume_batch (RdKafka::KafkaConsumer *, size_t, double);
RdKafka::Message * consume_single (RdKafka::KafkaConsumer *);
RdKafka::KafkaConsumer* Consumer(KafkaConfig);

#endif
