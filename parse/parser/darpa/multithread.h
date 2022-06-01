#ifndef SHADEWATCHER_PARSER_DARPA_MULTITHREAD_H_
#define SHADEWATCHER_PARSER_DARPA_MULTITHREAD_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <fstream>
#include "tripletdar.h"
#include "parser/kg.h"
#include "util/config.h"
#include "util/reduce_noise.h"

typedef std::vector<std::string> Batch;
typedef std::map<std::string, multi_t> IRinfo;
typedef struct ItemRepository {
    // batch buffer
    Batch * batch_buffer;
    multi_t batch_repo_size;
    multi_t batch_read_position;
    multi_t batch_write_position;
    multi_t batch_counter;        // record item count need to handle
    std::mutex batch_mtx;        //mutex lock for products buffer
    std::mutex batch_counter_mtx;
    std::condition_variable batch_repo_not_full;
    std::condition_variable batch_repo_not_empty;

    // graph buffer
    KG ** graph_buffer;
    multi_t graph_repo_size;
    multi_t graph_read_position;
    multi_t graph_write_position;
    multi_t graph_counter;
    std::mutex graph_mtx;
    std::condition_variable graph_repo_not_full;
    std::condition_variable graph_repo_not_empty;

    // batch info
    multi_t batch_size;
    multi_t batch_num;

}ItemRepository;

void InitItemRepository(ItemRepository &ir, std::string darpa_file, Config &cfg);
void ProduceBatch(ItemRepository &ir, Batch &item);
Batch& ConsumeBatch(ItemRepository &ir);
void ProduceBatchTask(ItemRepository &ir, std::string file_path);
void ConsumerTask(ItemRepository &ir, int dataset_type);
void SubKGConstruction(KG *infotbl, Batch item, int dataset_type);
void ProduceGraph(ItemRepository &ir, KG *infotbl);
KG * ConsumeGraph(ItemRepository &ir);
void MergeGraph(KG *infotbl_thread, KG *infotbl);
void ConsumeGraphTask(ItemRepository &ir, KG * infotbl);
multi_t BatchNum(event_t event_num, event_t batch_size);
event_t LineNum(std::string file_path);
void DelteIR(ItemRepository &ir);
void MultithreadConstruction(std::string darpa_file, KG *infotbl, Config &cfg);

#endif
