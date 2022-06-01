#include "parserkafka.h"

static volatile sig_atomic_t run = 1;

void SigTerm (int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    run = 0;
}

void KGKafkaParse(KG *infotbl, Config &cfg) {
	auto start = OverheadStart();

    // Create consumer
    KafkaConfig config = LoadKafkaConfig(cfg.kafka_config);
    std::string topic = config["topic"];
    std::cout << "kafka topic: " << topic << std::endl;
    RdKafka::KafkaConsumer *consumer = Consumer(config);

    Json::Value event;
    Triplet t(event, infotbl);
    int table_id = 0;

    // Signal to stop event processing
    signal(SIGINT, SigTerm);
    signal(SIGTERM, SigTerm);

    // batch_size and batch_tmout control how events are separated into files
    int batch_size = std::stoi(config["batch_size"]);
    double batch_tmout = std::stod(config["batch_tmout"]);

    // Consume messages in a streamming fashion
    while (run) {
        // read events in json format from topic
        auto msgs = consume_batch(consumer, batch_size, batch_tmout);

        for (auto msg = msgs.begin(); msg != msgs.end(); msg++) {
            event = msg->second;
            infotbl->event_num += 1;

            // ignore failed syscall (exception: EINPROGRESS in connect syscall)
            std::string status = Jval2str(event["auditd"]["result"]);
            if (status.compare("fail") == 0) {
                std::string ret_value = Jval2str(event["auditd"]["data"]["exit"]);
                if (ret_value.compare("EINPROGRESS") != 0) {
                    continue;
                }
            }

            // translate events into triplets
            if (event["auditd"]["data"].isMember("syscall")){
                t.Event2triplet();
            }
            else if (event["auditd"]["data"].isMember("acct")) {
                std::string op = Jval2str(event["auditd"]["data"]["op"]);
                if (op.compare("PAM:accounting") == 0) {
                    infotbl->InsertLoginProc(event, msgs, msg);
                }
            }
        }

        // recove names for processes without being executed
        infotbl->ProcInfoRecover();

        // compute e_id for each edges in KGEdgeList and create KGEdgeTable
        // we didn't compute e_id when creating edges becasue n_id may change
        // and using unordered map plus extract incur high overhead
        infotbl->GenKGEdgeTable();

        // reduce noise:
        ReduceNoise noise_reduction(infotbl);
        noise_reduction.DeleteNoise();

        // cleanup
        // 1. translate pipe nodes into pipe edges
        // 2. record external sockets -> delete local sockets
        infotbl->DeletePipeEdge();
        infotbl->DeleteLocalSocket();

        // print KG information
        infotbl->PrintKG();

        // store kg edges in database
        if(cfg.storetodb) {
			KGStoreEdgeToDB(cfg.postgres_config, infotbl, topic, table_id);
            table_id += 1;
		}

        // to save memory, we clean up edges/interactions in a KG after parsing an auditbeat file
        infotbl->edge_num += infotbl->KGEdgeTable.size();
        infotbl->FreeInteraction();
    }
    consumer->close();
    delete consumer;

    // store kg nodes in database
    if(cfg.storetodb) {
        KGStoreNodeToDB(cfg.postgres_config, infotbl, topic);
    }

	OverheadEnd(start, "KG construction");
}
