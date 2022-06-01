#include "parserdar.h"
#include "multithread.h"

int BatchSize(event_t logNum) {
	if (logNum <= 10000) {
		return 1;
	} else if (logNum <= 100000) {
		return 10;
	} else {
		return 100;
	}
}

// single-thread KG construction
void KGDarpaParse(std::string darpa_file, KG *infotbl, int dataset_type) {
	auto start = OverheadStart();
	
	// Read Log: compute offsets
	std::cout << "darpa file: " << darpa_file << std::endl;
	LogLoader *logload = new LogLoader(darpa_file);
	logload->LoadDARPALog();
	event_t event_num = logload->event_num;

	Json::Value event;
	Triplet t(event, infotbl, dataset_type);

	// progress bar parameters
	#ifdef BAR
		float progress = 0.0;
		int width = 50;
		int batchSize = BatchSize(event_num);
		int batchNum = event_num / batchSize;
		float addup = 1.0 / batchNum;
		int batchDeal = 0;
	#endif

	for (event_t id = 0; id < event_num; id++) {
		// Progress bar: high overhead (0.71s -> 0.93s)
		#ifdef BAR
			int newBatchDeal = id / batchSize;
			if (newBatchDeal > batchDeal) {
				ProgressBar(width, progress, addup);
				batchDeal = newBatchDeal;
			}
		#endif
		
		// read event in json format from audit logs
		logload->GetEvent(id, event);

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
	delete(logload);

	// Not all events in logload are analyzed
	// t.event_analyzed record #events before noise reduction
	infotbl->event_num += t.event_analyzed;

	// Reduce noise:
	// comment the following two lines to do the stat without noise reduction 
	ReduceNoise noise_reduction(infotbl);
	noise_reduction.DeleteNoise();

	// count original edges in a graph
	infotbl->edge_num += infotbl->KGEdgeTable.size();

	OverheadEnd(start, "KG construction");
}

void KGConstruction(std::string darpa_file, KG *infotbl, Config &cfg) {
	if (cfg.work_threads > 1) {
		// multi-thread KG construction (advanced multithread impelmentaion)
		MultithreadConstruction(darpa_file, infotbl, cfg);
	}
	else {
		KGDarpaParse(darpa_file, infotbl, cfg.dataset_type);
	}
}
