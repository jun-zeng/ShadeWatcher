#include "multithread.h"

// calculate batch number: batch number is the flag for thread to exit
multi_t BatchNum(event_t event_num, event_t batch_size) {
    event_t batch_num = 0;
    if (event_num % batch_size == 0) {
        batch_num = event_num / batch_size;
    } else {
        batch_num = (event_num / batch_size) + 1;
    }

    return batch_num;
}

// count the number of lines in a file
event_t LineNum(std::string file_path) {
    std::ifstream f_input(file_path, std::ios::in); 
    event_t event_num = 0;
    std::string line;
    while (std::getline(f_input, line)) {
        event_num++;
    }
    f_input.close();

    return event_num;
}

// this function is used to store one batch into batch buffer
void ProduceBatch(ItemRepository &ir, Batch &item) {

    // std::mutex lock to ensure only one thread can visit the batch buffer at one time
    std::unique_lock<std::mutex> lock(ir.batch_mtx);

    // if the batch buffer is full, wait until it has space
    while (((ir.batch_write_position + 1) % ir.batch_repo_size) == ir.batch_read_position) {
        (ir.batch_repo_not_full).wait(lock);
    }

    std::swap((ir.batch_buffer)[ir.batch_write_position], item);
    (ir.batch_write_position)++;
    
    if (ir.batch_write_position == ir.batch_repo_size) {
        ir.batch_write_position = 0;
    }

    (ir.batch_repo_not_empty).notify_all();
    lock.unlock();
}

// read one batch from batch buffer
Batch& ConsumeBatch(ItemRepository &ir) {

    // use lock to ensure only one thread can visit the batch buffer at one time
    std::unique_lock<std::mutex> lock(ir.batch_mtx);

    while (ir.batch_read_position == ir.batch_write_position) {
        (ir.batch_repo_not_empty).wait(lock);
    }

    Batch &item = (ir.batch_buffer)[ir.batch_read_position];
    (ir.batch_read_position)++;

    if (ir.batch_read_position >= ir.batch_repo_size) {
        ir.batch_read_position = 0;
    }

    (ir.batch_repo_not_full).notify_all();
    lock.unlock();

    return item;
}

// read initial data, split it to batch, and store it in the batch buffer
void ProduceBatchTask(ItemRepository &ir, std::string file_path) {
    std::ifstream f_input(file_path, std::ios::in);
    std::string line;

    Batch batch;
    while (std::getline(f_input, line)) {
        batch.push_back(line);
        if (batch.size() == ir.batch_size) {
            ProduceBatch(ir, batch);
        }
    }
    if (batch.size() > 0) {
        ProduceBatch(ir, batch);
    }
    f_input.close();
}

// this function is used to parse batch and construct subKGgraph, then store it in graph buffer
void ConsumerTask(ItemRepository &ir, int dataset_type) {
    while (1) {
        Batch batch;
        // use lock to ensure only one thread visits the batch buffer
        std::unique_lock<std::mutex> lock(ir.batch_counter_mtx);
        // after parsing all batch, exit
        if (ir.batch_counter < ir.batch_num) {
            std::swap(batch, ConsumeBatch(ir));
            ir.batch_counter += 1;
        } else {
            break;
        }
        
        lock.unlock();
        
        // construct subKGgraph with one batch
        KG * infotbl = new KG(darpa);
        SubKGConstruction(infotbl, batch, dataset_type);
        // Batch().swap(batch);
        ProduceGraph(ir, infotbl);
    }
}

// store one subGraph into graph buffer
void ProduceGraph(ItemRepository &ir, KG *infotbl) {
    std::unique_lock<std::mutex> lock(ir.graph_mtx);
    while (((ir.graph_write_position + 1) % ir.graph_repo_size) == ir.graph_read_position) {
        (ir.graph_repo_not_full).wait(lock);
    }

    (ir.graph_buffer)[ir.graph_write_position] = infotbl;
    (ir.graph_write_position)++;

    if (ir.graph_write_position == ir.graph_repo_size) {
        ir.graph_write_position = 0;
    }

    // when there are subgraph waiting for parsing, notify other thread to parse it.
    (ir.graph_repo_not_empty).notify_all();
    lock.unlock();

}

// visit the graph buffer and obtain one subGraph
KG * ConsumeGraph(ItemRepository &ir) {
    std::unique_lock<std::mutex> lock(ir.graph_mtx);

    while (ir.graph_read_position == ir.graph_write_position) {
        (ir.graph_repo_not_empty).wait(lock);
    }

    KG * infotbl = (ir.graph_buffer)[ir.graph_read_position];
    (ir.graph_read_position)++;

    if (ir.graph_read_position >= ir.graph_repo_size) {
        ir.graph_read_position = 0;
    }

    (ir.graph_repo_not_full).notify_all();
    lock.unlock();
    
    return infotbl;
}

// parse subGraph and merge subGraph
void ConsumeGraphTask(ItemRepository &ir, KG * infotbl) {
    while (1) {
        if (ir.graph_counter < ir.batch_num) {
            KG * infotbl_thread = ConsumeGraph(ir);
            MergeGraph(infotbl_thread, infotbl);
            ir.graph_counter  += 1;
        } else {
            break;
        }
    }
}

// init the ItemRepository
void InitItemRepository(ItemRepository &ir, std::string darpa_file, Config &cfg) {

    IRinfo ir_info = LoadMultithreadConfig(cfg.multithread_config);

    ir.batch_read_position = 0;
    ir.batch_write_position = 0;
    ir.batch_counter = 0;
    ir.graph_read_position = 0;
    ir.graph_write_position = 0;
    ir.graph_counter = 0;
    ir.batch_size = ir_info["batch_size"];
    ir.batch_num = BatchNum(LineNum(darpa_file), ir_info["batch_size"]);
    ir.batch_repo_size = ir_info["batch_repo_size"];
    ir.graph_repo_size = ir_info["graph_repo_size"];
    ir.batch_buffer = new Batch[ir_info["batch_repo_size"]];
    ir.graph_buffer = new KG *[ir_info["graph_repo_size"]];
}

void DelteIR(ItemRepository &ir) {
    delete [] ir.batch_buffer;
    delete [] ir.graph_buffer;
}
// KG construction for one batch
void SubKGConstruction(KG *infotbl, Batch batch, int dataset_type) {
    Json::Value event;
    Triplet t(event, infotbl, dataset_type);
    
    for (auto line : batch) {
        Json::CharReaderBuilder builder {};
        auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

        JSONCPP_STRING errs;
        const auto parseSuccessful = reader->parse(line.c_str(),
                                                    line.c_str() + line.length(),
                                                    &event,
                                                    &errs);
        if (!parseSuccessful) {
            std::cerr << "Fail to read batch data." << std::endl;
        }
        // translate events into triplets
		if (event["datum"].isMember("com.bbn.tc.schema.avro.cdm18.Event")){
			t.Event2triplet();
		} else if (event["datum"].isMember("com.bbn.tc.schema.avro.cdm18.Subject")){
			t.LoadProc();
		} else if (event["datum"].isMember("com.bbn.tc.schema.avro.cdm18.FileObject")){
			t.LoadFile();
		} else if (event["datum"].isMember("com.bbn.tc.schema.avro.cdm18.NetFlowObject")) {
			t.LoadSock();
		}
    }
    
    infotbl->event_num = t.event_analyzed;
}

//  merge subGraph
void MergeGraph(KG *infotbl_thread, KG *infotbl) {
    infotbl->event_num += infotbl_thread->event_num;

	for (auto it: infotbl_thread->ProcNodeTable) {
		hash_t p_hash = it.first;
		NodeProc *p = it.second;
		infotbl->ProcNodeTable[p_hash] = p;
		infotbl->KGNodeTable[p_hash] = NodeType_t::Proc;
	}

	for (auto it: infotbl_thread->FileNodeTable) {
		hash_t f_hash = it.first;
		NodeFile *f = it.second;
		infotbl->FileNodeTable[f_hash] = f;
		infotbl->KGNodeTable[f_hash] = NodeType_t::File;
	}

	for (auto it: infotbl_thread->SocketNodeTable) {
		hash_t s_hash = it.first;
		NodeSocket *s = it.second;
		infotbl->SocketNodeTable[s_hash] = s;
		infotbl->KGNodeTable[s_hash] = NodeType_t::Socket;
	}

	for (auto it: infotbl_thread->NoiseTable) {
		hash_t n_hash = it.first;
		NodeType_t n_type = it.second;
		infotbl->NoiseTable[n_hash] = n_type;
	}

	for (auto it: infotbl_thread->KGEdgeTable) {
        hash_t e_id = it.first;
        KGEdge *e = it.second;
		infotbl->InsertEdge(e_id, e);
	}

	for (auto it: infotbl_thread->FileInteractionTable) {
		hash_t f_hash = it.first;
		inter_map *im = it.second;
		infotbl->InsertFileInteraction(f_hash, im);
	}

	for (auto it: infotbl_thread->ProcInteractionTable) {
		hash_t p_hash = it.first;
		inter_map *im = it.second;
		infotbl->InsertProcInteraction(p_hash, im);
	}

	delete(infotbl_thread);
}

// start threads
void MultithreadConstruction(std::string darpa_file, KG *infotbl, Config &cfg) {
    auto start = OverheadStart();

    // Read Log: compute offsets
	std::cout << "darpa file: " << darpa_file << std::endl;
    
    ItemRepository ir;
	InitItemRepository(ir, darpa_file, cfg);
    
    std::vector<std::thread> thread_vector;

    thread_vector.push_back(std::thread(ProduceBatchTask, std::ref(ir), darpa_file));
    for (thread_t i = 0; i < cfg.work_threads; i++) {
        thread_vector.push_back(std::thread(ConsumerTask, std::ref(ir), cfg.dataset_type));
    }
    thread_vector.push_back(std::thread(ConsumeGraphTask, std::ref(ir), infotbl));
    for (auto &_thread : thread_vector) {
        _thread.join();
    }
    DelteIR(ir);

    // Reduce noise:
	// comment the following two lines to do the stat without noise reduction 
	ReduceNoise noise_reduction(infotbl);
	noise_reduction.DeleteNoise();

    // count original edges in a graph
	infotbl->edge_num += infotbl->KGEdgeTable.size();

    OverheadEnd(start, "KG construction");
}
