#include "config.h"

Config::Config(int argc, char **argv): CmdArgument(argc, argv) {
    graph = false;

    // darpa driver
    loadgraph = false;
	work_threads = 1;
	storegraph = false;
}

void Config::EnsureDir(std::string dir_path) {
	struct stat buffer;
	if ((stat (dir_path.c_str(), &buffer) == 0) == false) {
		std::cout << "Dir: " << dir_path << " does not exist before!" << std::endl;
		std::string sub_dir_path = dir_path.substr(0, dir_path.rfind("/"));
		EnsureDir(sub_dir_path);
		int check = mkdir(dir_path.c_str(), 0775);
		if (!check)
			std::cout << "Create a new dir: " << dir_path << std::endl;
		else 
			std::cout << "Create dir failed!" << std::endl;
	}
}

bool Config::TestConfigFile(std::string& conf_path) {
	struct stat buffer;
	if ((stat (conf_path.c_str(), &buffer) == 0) == false) {
		return false;
	}
	else {
		return true;
	}
}

void Config::ConfigDar() {
    // display help information
	if(ComOptionExist("-h") || ComOptionExist("-help")) {
		ComDarHelp();
		exit(EXIT_SUCCESS);
	}

 	// delete a schema in a database
	if (ComOptionExist("-delschema")) {
		delschema_flag = true;
		delschema = GetComOption("-delschema");
		postgres_config = GetSecComOption("-delschema");
	}
    else if (ComOptionExist("-dsch")) {
		delschema_flag = true;
        delschema = GetComOption("-dsch");
		postgres_config = GetSecComOption("-dsch");
    }
	if (delschema_flag == true) {
		if (delschema.empty() == true || postgres_config == "") {
			std::cout << "Input the schema and database to delete!" << std::endl;
			exit(EXIT_FAILURE);
		}
		postgres_config = "../config/" + postgres_config;
		return;
	}

	// define darpa log dir path (e.g., ../data/darpa/e3/trace/b/ta1-trace-e3-official-1.json)
	if(ComOptionExist("-trace")) {
		trace = true;
		darpa_data_dir = GetComOption("-trace");
	}
    else if (ComOptionExist("-t")) {
		trace = true;
        darpa_data_dir = GetComOption("-t");
    }

    // define ont-hot encoding/embedding directory 
	if (ComOptionExist("-dataset")) {
		darpa_dataset = GetComOption("-dataset");
	}
    else if (ComOptionExist("-da")) {
        darpa_dataset = GetComOption("-da");
    }
	else {
		std::cerr << "Declare darpa dataset before encoding" << std::endl;
		exit(EXIT_FAILURE);
	}
	// embed_data_path: store one-hot encoding/kg information
	// embed_res_path: store system entity and relation embeddings
	embed_data_path = "../data/encoding/" + darpa_dataset;
	EnsureDir(embed_data_path);
	embed_res_path = "../data/embedding/" + darpa_dataset;
	EnsureDir(embed_res_path);

	// Trace or Theia: different darpa teams have different log format
	dataset_type = IdentifyDataset(darpa_dataset);

	// decide the number of threads to construct KG (by default one thread)
	std::string multithread;
	if(ComOptionExist("-multithread")) {
		multithread = GetComOption("-multithread");
		if ((multithread != "") && (multithread[0] != '-')) {
			work_threads = std::stoi(multithread);
		}
		else {
			work_threads = std::thread::hardware_concurrency();
		}
	}
    else if (ComOptionExist("-m")) {
        multithread = GetComOption("-m");
		if ((multithread != "") && (multithread[0] != '-')) {
			work_threads = std::stoi(multithread);
		}
		else {
			work_threads = std::thread::hardware_concurrency();
		}
    }

	// configure file for multi-thread (e.g., multithread.cfg)
	if(ComOptionExist("-threadconfig")) {
		multithread_config = "../config/" + GetComOption("-threadconfig");
	}
	else if(ComOptionExist("-t")) {
		multithread_config = "../config/" + GetComOption("-t");
	}

	// database load or store flag 
	loadfromdb = ComOptionExist("-ldb") || ComOptionExist("-loadfromdb");
	storetodb = ComOptionExist("-sdb") || ComOptionExist("-storetodb");
	if (loadfromdb && storetodb) {
		std::cout << "Choose to either load/store data" << std::endl;
		exit(EXIT_FAILURE);
	}

	// collect database config to load KG
	if (loadfromdb) {
		if (ComOptionExist("-loadfromdb")) {
			edg_ld_files = GetComOption("-loadfromdb");
			// whether load all KGs from db
			if (edg_ld_files == "all") {
				ld_all_edg_flag = true;	
			}
			postgres_config = GetSecComOption("-loadfromdb");			
		}
    	else if (ComOptionExist("-ldb")) {
        	edg_ld_files = GetComOption("-ldb");
			if (edg_ld_files == "all") {
				ld_all_edg_flag = true;	
			}
			postgres_config = GetSecComOption("-ldb");			
    	}
		if (edg_ld_files.empty() == true || postgres_config.empty() == true) {
			std::cout << "Input darpa file and dataset to load" << std::endl;
			exit(EXIT_FAILURE);
		}
		postgres_config = "../config/" + postgres_config;
	}

	// collect database config to store KG
	if (storetodb) {
		if (GetComOption("-storetodb") == "") {
			postgres_config = "../config/" + GetComOption("-sdb");
		}
		else {
			postgres_config = "../config/" + GetComOption("-storetodb");
		}
		if(TestConfigFile(postgres_config) == false) {
			std::cout << "Cannot find " << postgres_config << "to connect to the database!" << std::endl;
			exit(EXIT_FAILURE);
		}	
	}

	// file load or store flag 
	loadfromfile = ComOptionExist("-lf") || ComOptionExist("-loadfile");
	storetofile = ComOptionExist("-sf") || ComOptionExist("-storefile");
	storeentity = ComOptionExist("-se") || ComOptionExist("-storeentity");
	loadentity = ComOptionExist("-le") || ComOptionExist("-loadentity");

	if (loadfromfile && storetofile) {
		std::cout << "Choose to either load/store data" << std::endl;
		exit(EXIT_FAILURE);
	}

	// KG visualization
	if (ComOptionExist("-g") || ComOptionExist("-graph")) {
		graph = true;
	}

	if (!(trace || loadfromfile || loadfromdb || loadentity)) {
		std::cout << "Input log dir/file using -trace XXX or Load graph from db/file" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Config::ConfigKafka() {
	// display help information
	if(ComOptionExist("-h") || ComOptionExist("-help")) {
		ComKafkaHelp();
		exit(EXIT_SUCCESS);
	}

	// define audit log dir path (e.g., ../data/examples/nano_scp_1)
	if(ComOptionExist("-trace")) {
		trace = true;
		auditbeat_data_dir = GetComOption("-trace");
	}
    else if (ComOptionExist("-t")) {
		trace = true;
        auditbeat_data_dir = GetComOption("-t");
    }
	if (auditbeat_data_dir.back() == '/') {
		auditbeat_data_dir.pop_back();
	}

	// configure file for kafka (e.g., kafka.cfg)
	if(ComOptionExist("-kafkaconfig")) {
		kafka_config = "../config/" + GetComOption("-kafka");
	}
	else if(ComOptionExist("-k")) {
		kafka_config = "../config/" + GetComOption("-t");
	}

	// KG visualization
	if (ComOptionExist("-g") || ComOptionExist("-graph")) {
		graph = true;
	}

	// database load or store flag 
	loadfromdb = ComOptionExist("-ldb") || ComOptionExist("-loadfromdb");
	storetodb = ComOptionExist("-sdb") || ComOptionExist("-storetodb");
	if (loadfromdb && storetodb) {
		std::cout << "Choose to either load/store data" << std::endl;
		exit(EXIT_FAILURE);
	}

	// collect database config to load KG
	if (loadfromdb) {
		if (GetComOption("-loadfromdb") == "") {
			postgres_config = "../config/" + GetComOption("-ldb");
		}
		else {
			postgres_config = "../config/" + GetComOption("-loadfromdb");
		}
		if(TestConfigFile(postgres_config) == false) {
			std::cout << "Cannot find " << postgres_config << "to connect to the database!" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (GetNextComOption("-loadfromdb") == "") {
			topic = GetNextComOption("-ldb");
		}
		else {
			topic = GetNextComOption("-loadfromdb");
		}
		if (topic == "" or topic.at(0) == '-') {
			std::cout << "Please define topic to load events " << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// collect database config to store KG
	if (storetodb) {
		if (GetComOption("-storetodb") == "") {
			postgres_config = "../config/" + GetComOption("-sdb");
		}
		else {
			postgres_config = "../config/" + GetComOption("-storetodb");
		}
		if(TestConfigFile(postgres_config) == false) {
			std::cout << "Cannot find " << postgres_config << "to connect to the database!" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	if (!(trace || loadfromdb)) {
		std::cout << "Input log dir using -trace XXX or loadfromdb" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

void Config::ConfigBeat() {
    // display help information
	if(ComOptionExist("-h") || ComOptionExist("-help")) {
		ComBeatHelp();
		exit(EXIT_SUCCESS);
	}

 	// delete a schema in a database
	if (ComOptionExist("-delschema")) {
		delschema_flag = true;
		delschema = GetComOption("-delschema");
		postgres_config = GetSecComOption("-delschema");
	}
    else if (ComOptionExist("-dsch")) {
		delschema_flag = true;
        delschema = GetComOption("-dsch");
		postgres_config = GetSecComOption("-dsch");
    }
	if (delschema_flag == true) {
		if (delschema.empty() == true || postgres_config.empty() == true) {
			std::cout << "Input the schema and database to delete!" << std::endl;
			exit(EXIT_SUCCESS);
		}
		postgres_config = "../config/" + postgres_config;
		return;
	}

	// define audit log dir path (e.g., ../data/examples/nano_scp_1)
	if(ComOptionExist("-trace")) {
		trace = true;
		auditbeat_data_dir = GetComOption("-trace");
	}
    else if (ComOptionExist("-t")) {
		trace = true;
        auditbeat_data_dir = GetComOption("-t");
    }
	if (auditbeat_data_dir.back() == '/') {
		auditbeat_data_dir.pop_back();
	}

    // define ont-hot encoding/embedding directory 
	if (ComOptionExist("-dataset")) {
		auditbeat_dataset = GetComOption("-dataset");
		dataset = true;
	}
    else if (ComOptionExist("-da")) {
        auditbeat_dataset = GetComOption("-da");
		dataset = true;
    }
	else {
		auditbeat_dataset = "log";
	}
	// embed_data_path: store one-hot encoding/kg information
	// embed_res_path: store system entity and relation embeddings
	embed_data_path = "../data/encoding/" + auditbeat_dataset;
	EnsureDir(embed_data_path);
	embed_res_path = "../data/embedding/" + auditbeat_dataset;
	EnsureDir(embed_res_path);

	// database load or store flag 
	loadfromdb = ComOptionExist("-ldb") || ComOptionExist("-loadfromdb");
	storetodb = ComOptionExist("-sdb") || ComOptionExist("-storetodb");
	if (loadfromdb && storetodb) {
		std::cout << "Choose to either load/store data" << std::endl;
		exit(EXIT_FAILURE);
	}

	// collect database config to load KG
	if (loadfromdb) {
		if (GetComOption("-loadfromdb") == "") {
			postgres_config = "../config/" + GetComOption("-ldb");
		}
		else {
			postgres_config = "../config/" + GetComOption("-loadfromdb");
		}
		if(TestConfigFile(postgres_config) == false) {
			std::cout << "Cannot find " << postgres_config << "to connect to the database!" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// collect database config to store KG
	if (storetodb) {
		if (GetComOption("-storetodb") == "") {
			postgres_config = "../config/" + GetComOption("-sdb");
		}
		else {
			postgres_config = "../config/" + GetComOption("-storetodb");
		}
		if(TestConfigFile(postgres_config) == false) {
			std::cout << "Cannot find " << postgres_config << "to connect to the database!" << std::endl;
			exit(EXIT_FAILURE);
		}	
	}

	// file load or store flag 
	loadfromfile = ComOptionExist("-lf") || ComOptionExist("-loadfile");
	storeentity = ComOptionExist("-se") || ComOptionExist("-storeentity");
	storetofile = ComOptionExist("-sf") || ComOptionExist("-storefile");
	loadentity = ComOptionExist("-le") || ComOptionExist("-loadentity");

	if (loadfromfile && storetofile) {
		std::cout << "Choose to either load/store data" << std::endl;
		exit(EXIT_FAILURE);
	}

	// KG visualization
	if (ComOptionExist("-g") || ComOptionExist("-graph")) {
		graph = true;
	}

	if (!(trace || loadfromfile || loadfromdb || loadentity)) {
		std::cout << "Input log dir/file using -trace XXX or Load graph" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

std::map<std::string,std::string> LoadPostgresConfig(std::string config_file) {
	libconfig::Config cfg;
	try {
		cfg.readFile(config_file.c_str());
	} catch (libconfig::FileIOException &e) {
		std::cerr << "Fail to read file: " << config_file << std::endl;
		exit(EXIT_FAILURE);
	}

	// read configuration from config file
	std::string username = cfg.lookup("username");
	std::string host = cfg.lookup("host");
	std::string password = cfg.lookup("password");
	std::string dbname = cfg.lookup("dbname");
	std::string port = cfg.lookup("port");
	std::string batch_node = cfg.lookup("batch_node");
	std::string batch_edge = cfg.lookup("batch_edge");

	std::map<std::string,std::string> db_config;
	db_config["username"] = username;
	db_config["host"] = host;
	db_config["password"] = password;
	db_config["dbname"] = dbname;
	db_config["port"] = port;
	db_config["batch_node"] = batch_node;
	db_config["batch_edge"] = batch_edge;

	return db_config;
}

std::map<std::string,std::string> LoadNeo4jConfig(std::string config_file) {
	libconfig::Config cfg;
	try {
		cfg.readFile(config_file.c_str());
	} catch (libconfig::FileIOException &e) {
		std::cerr << "Fail to read file: " << config_file << std::endl;
		exit(EXIT_FAILURE);
	}

	// read configuration from config file
	std::string url = cfg.lookup("url");
	std::string batch_edge = cfg.lookup("batch_edge");
	std::string batch_node = cfg.lookup("batch_node");

	std::map<std::string,std::string> db_config;
	db_config["url"] = url;
	db_config["batch_edge"] = batch_edge;
	db_config["batch_node"] = batch_node;

	return db_config;
}

std::map<std::string, std::string> LoadKafkaConfig(std::string config_file) {
	libconfig::Config cfg;

	try {
		cfg.readFile(config_file.c_str());
		std::cout << "Kafka Configure file: " << config_file << std::endl;
	} catch (libconfig::FileIOException &e) {
		std::cerr << "Fail to read file: " << config_file << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string broker = cfg.lookup("broker");
	std::string topic = cfg.lookup("topic");
	std::string group_id = cfg.lookup("group_id");
	std::string batch_size = cfg.lookup("batch_size");
	std::string batch_tmout = cfg.lookup("batch_tmout");

	std::map<std::string, std::string> kafka_config;

	kafka_config["broker"] = broker;
	kafka_config["topic"] = topic;
	kafka_config["group_id"] = group_id;
	kafka_config["batch_size"] = batch_size;
	kafka_config["batch_tmout"] = batch_tmout;

	return kafka_config;
}

std::map<std::string, multi_t> LoadMultithreadConfig(std::string config_file) {
	libconfig::Config cfg;

	try {
		cfg.readFile(config_file.c_str());
		std::cout << "Multi-thread Configure file: " << config_file << std::endl;
	} catch (libconfig::FileIOException &e) {
		std::cerr << "Fail to read file: " << config_file << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string batch_size = cfg.lookup("batch_size");
	std::string batch_repo_size = cfg.lookup("batch_repo_size");
	std::string graph_repo_size = cfg.lookup("graph_repo_size");

	std::map<std::string, multi_t> multithread_config;
	multithread_config["batch_size"] = std::stoul(batch_size);
	multithread_config["batch_repo_size"] = std::stoul(batch_repo_size);
	multithread_config["graph_repo_size"] = std::stoul(graph_repo_size);

	return multithread_config;
}

int IdentifyDataset(std::string darpa_dataset) {
	if (darpa_dataset.find("trace") != std::string::npos) {
		return DARPA_TRACE;
	}
	else if (darpa_dataset.find("theia") != std::string::npos) {
		return DARPA_THEIA;
	}
	else {
		 std::cerr << "Unknown darpa dataset: " << darpa_dataset << " " << __FILE__ << " " <<  __LINE__<< std::endl;
		 exit(EXIT_FAILURE);
	}
}
