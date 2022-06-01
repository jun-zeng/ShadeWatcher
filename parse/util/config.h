#ifndef SHADEWATCHER_UTIL_CONFIG_H_
#define SHADEWATCHER_UTIL_CONFIG_H_

#define DARPA_TRACE 0
#define DARPA_THEIA 1

#include <libconfig.h++>
#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <sys/stat.h>
#include "parser/common.h"
#include "cmdargument.h"

class Config: public CmdArgument
{
public:
    bool graph;
    bool delschema_flag = false;
    bool ld_all_edg_flag = false;
    bool storeentity = false;
    bool loadentity = false;
    
    std::string edg_ld_files;
    std::string delschema;
    std::string embed_data_path;
    std::string embed_res_path;
    std::string postgres_config;
    std::string neo4j_config = "../config/neo4jdb.cfg";

    // darpa driver
    std::string darpa_dataset;
    std::string darpa_data_dir;
    bool loadgraph;
    thread_t work_threads;
    bool storegraph;
    bool storetodb = false;
    bool storetofile = false;
    bool loadfromdb = false;
    bool loadfromfile = false;
    bool trace = false;
    bool dataset = false;
    int dataset_type;
    std::string multithread_config = "../config/multithread.cfg";
    std::string kafka_config = "../config/kafka.cfg";

    // auditbeat driver
    std::string auditbeat_data_dir;
    std::string auditbeat_dataset;

    // kafka driver
    std::string topic;

    Config(int, char **);
    void EnsureDir(std::string);
    bool TestConfigFile(std::string&);
    void ConfigDar();
    void ConfigBeat();
    void ConfigKafka();
};

std::map<std::string,std::string> LoadPostgresConfig(std::string);
std::map<std::string,std::string> LoadNeo4jConfig(std::string);
std::map<std::string, multi_t> LoadMultithreadConfig(std::string);
std::map<std::string, std::string> LoadKafkaConfig(std::string);

int IdentifyDataset(std::string);

#endif
