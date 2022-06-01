#include "kafka/consumer.h"

// read a batch (batch_size) of messages or wait for batch_tmout seconds
std::map<seq_t, Json::Value> consume_batch (RdKafka::KafkaConsumer *consumer, 
                                              size_t batch_size, 
                                              double batch_tmout) {
    clock_t c_start = clock();

    std::map<seq_t, Json::Value> msgs;
    while (msgs.size() < batch_size) {
        RdKafka::Message *msg = consumer->consume(batch_tmout * 100);
        std::string msg_str;
        seq_t seq;

        Json::Value msg_jval;
        Json::CharReaderBuilder builder {};
        auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
        JSONCPP_STRING errs;

        switch (msg->err()) {
            case RdKafka::ERR__TIMED_OUT:
                delete msg;
                return msgs;

            case RdKafka::ERR_NO_ERROR:
                msg_str =  static_cast<const char *>(msg->payload());
                
                // convert msg_str into a json object
                if (!reader->parse(msg_str.c_str(), msg_str.c_str() + msg_str.length(),
                    &msg_jval, &errs)) {
                        std::cerr << "Fail to parse a topic" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                seq = Jval2int(msg_jval["auditd"]["sequence"]);
                msgs[seq] = msg_jval;
                delete msg;
                break;

            default:
                std::cerr << "%% Consumer error: " << msg->errstr() << std::endl;
                delete msg;
                return msgs;
        }

        clock_t c_end = clock();
        double duration = ((double)(c_end-c_start))/CLOCKS_PER_SEC;
        if (duration > batch_tmout) {
            break;
        }
    }
    return msgs;
}

// read a single message or wait for batch_tmout seconds
RdKafka::Message * consume_single (RdKafka::KafkaConsumer *consumer) {    
    RdKafka::Message *msg = consumer->consume(80);

    switch (msg->err()) {
        case RdKafka::ERR__TIMED_OUT:
            delete msg;
            return NULL;

        case RdKafka::ERR_NO_ERROR:
            return msg;

        default:
            std::cerr << "%% Consumer error: " << msg->errstr() << std::endl;
            delete msg;
            return NULL;
    }
}

// Initialize Consumer (broker, topic, groupid...)
RdKafka::KafkaConsumer* Consumer(KafkaConfig config) {
    // by default, localhost:9092
    std::string broker = config["broker"];

    // by default, log
    std::vector<std::string> topics; 
    std::string topic = config["topic"];
    topics.push_back(topic);

    // by default, auditbeat
    std::string group_id = config["group_id"];

    // Set up configuration
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::string errstr;
    if (conf->set("enable.auto.commit", "false", errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }
    if (conf->set("auto.offset.reset", "earliest", errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }
    if (conf->set("enable.partition.eof", "false", errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }
    if (conf->set("bootstrap.servers", broker, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }
    if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }

    // Create consumer
    RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(conf, errstr);
    if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        exit(1);
    }
    delete conf;

    // Subscribe to topics
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if (err) {
    std::cerr << "Failed to subscribe to " << topics.size() << " topics: "
            << RdKafka::err2str(err) << std::endl;
    exit(1);
    }

    // signal(SIGINT, sigterm);
    // signal(SIGTERM, sigterm);

    // Consume messages in batches of batch_size
    // while (run) {
    //     auto msgs = consume_batch(consumer, 1, batch_tmout);
    //     std::cout << "Accumulated " << msgs.size() << " messages:" << std::endl;

    //     for (auto &msg : msgs) {
    //         // std::cout << " Message in " << msg->topic_name() << " [" << msg->partition() << "] at offset " << msg->offset() << std::endl;
    //         std::cout << static_cast<const char *>(msg->payload()) << std::endl;
    //         delete msg;
    //     }
    // }

    return consumer;

    // Close and destroy consumer
    // consumer->close();
    // delete consumer;
}
