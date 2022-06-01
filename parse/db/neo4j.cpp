#include "neo4j.h"

Neo4jdb::Neo4jdb(std::string config_file, KG *_infotbl) {
	infotbl = _infotbl;
	neo4j_client_init();
	std::map<std::string, std::string> cfg = LoadNeo4jConfig(config_file);

	std::string url = cfg["url"];
	connection = neo4j_connect(url.c_str(), NULL, NEO4J_INSECURE);
	if (connection == NULL) {
		neo4j_perror(stderr, errno, "Neo4j Connection failed");
	}
	batch_edge = std::stoi(cfg["batch_edge"]);
	batch_node = std::stoi(cfg["batch_node"]);
}

Neo4jdb::~Neo4jdb() {
	neo4j_close(connection);
	neo4j_client_cleanup();
}

std::string Neo4jdb::run(std::string cmd) {
	neo4j_result_stream_t *results;
	neo4j_result_t *result;
	neo4j_value_t value;

	char *run_cmd = new char[cmd.length() + 1];
	strcpy(run_cmd, cmd.c_str());

	results = neo4j_run(connection, run_cmd, neo4j_null);	
	result = neo4j_fetch_next(results);
	value = neo4j_result_field(result, 0);

	char value_char[128];
	neo4j_tostring(value, value_char, sizeof(value_char));
	std::string value_str = std::string(value_char);

	delete [] run_cmd;
	neo4j_close_results(results);

	return value_str;
}

void Neo4jdb::Cleandb() {
	run("match (n)-[r]-(m) delete n,r,m");
	run("match (n) delete n");
}

std::string Neo4jdb::Neo4jSearchNode(hash_t n_hash) {
	std::string cmd;
	auto it = infotbl->KGNodeTable.find(n_hash);
	if (it == infotbl->KGNodeTable.end()){
		std::cerr << "Neo4j miss node" + std::to_string(n_hash);
		return "";
	}

	switch (it->second){
		case NodeType_t::Proc:{
			NodeProc *p = infotbl->SearchProc(n_hash);
			std::string p_hash = std::to_string(n_hash);			
			cmd = NodeEnum2String(NodeType_t::Proc) + 
			"{pid:\"" + p->pid + "\", exe:\"" + p->exe + "\", args:\"" + p->args + "\",hash:\"" + p_hash + "\"}";
			break;
		}
		case NodeType_t::File:{
			NodeFile *f = infotbl->SearchFile(n_hash);
			std::string f_hash = std::to_string(n_hash);
			cmd = NodeEnum2String(NodeType_t::File) + "{name:\"" + f->name + "\", hash:\"" + f_hash + "\"}";
			break;
		}
		case NodeType_t::Socket:{
			NodeSocket *s = infotbl->SearchSocket(n_hash);
			std::string s_hash = std::to_string(n_hash);
			cmd = NodeEnum2String(NodeType_t::Socket) + "{name:\"" + s->name + "\", hash:\"" + s_hash + "\"}";
			break;
		}
		case NodeType_t::Attr:{
			// NodeAttr *a = infotbl->SearchAttr(n_hash);
			// std::string a_hash = std::to_string(*(a->id));
			break;
		}
		case NodeType_t::NotDefined:{
			break;
		}
	}

	return cmd;
}

bool Neo4jdb::Createnode(std::string cmd_n) {
	// do not visulize missing node
	if (cmd_n == "") {
		return false;
	}

	std::string cmd = "MERGE (m: " + cmd_n + ")";
	std::string value = run(cmd);
	// std::cout << cmd << std::endl;

	if (value.empty()) {
		return false;
	}
	else {
		return true;
	}
}

std::string Neo4jdb::NodeCmd(hash_t n_hash, nodenum_t n_seq) {
	std::string n_info = Neo4jSearchNode(n_hash);
	if (n_info == "") {
		return "";
	}
	std::string n_str = "n" + std::to_string(n_seq);
	std::string n_cmd = "CREATE (" + n_str + ": " + n_info + ") ";
	return n_cmd;
}

void Neo4jdb::Neo4jVisKG() {
	std::cout << "\nVisualize KG" << std::endl;
	// cleanup neo4j before visulization
	Cleandb();
	std::string cmd;

	// store nodes into neo4j
	event_t n_count = 0;
	for (auto it: infotbl->KGNodeTable) {
		hash_t n_hash = it.first;
		auto n_cmd = NodeCmd(n_hash, n_count);
		if (n_cmd == "") {
			db_print("Fail to create nodes");
			continue;
		}

		cmd += n_cmd;
		if (n_count != batch_node) {
			n_count ++;
		}
		if (n_count == batch_node) {
			run(cmd);
			cmd.clear();
			n_count = 0;
		}
	}
	// In case that #node cannot be divided by #batch_node
	// If #node can be divided by #batch_node, n_count == 0
	if (n_count) {
		run(cmd);
		cmd.clear();
	}

	// store edges into neo4j
	event_t e_count = 0;
	for (auto &it: infotbl->KGEdgeTable) {
		KGEdge* edge = it.second;

		// Create new edge in db
		auto e_cmd = EdgeCmd(edge, e_count);
		cmd += e_cmd;

		if (e_count != batch_edge) {
			e_count ++;
		}
		if (e_count == batch_edge) {
			// std::cout << cmd << std::endl;
			auto pos = cmd.rfind("with");
			cmd.replace(pos, cmd.length(), "");
			run(cmd);
			cmd.clear();
			e_count = 0;
		}
	}

	// In case that #edge cannot be divided by #batch
	// If #node can be divided by #batch, count == 0
	if (e_count) {
		auto pos = cmd.rfind("with");
		cmd.replace(pos, cmd.length(), "");
		run(cmd);
	}
}

void Neo4jdb::Neo4jVizEdge(std::vector<KGEdge*> edges) {
	// cleanup neo4j before visulization
	Cleandb();

	std::string cmd;
	for (const auto &edge: edges) {
		std::string n1_info = Neo4jSearchNode(edge->n1_hash);
		std::string n2_info = Neo4jSearchNode(edge->n2_hash);
		if (n1_info == "" or n2_info == "") {
			continue;
		}

		// create now nodes; we use merge instead of create becasue 
		// nodes can be redundant
		std::string n1_cmd = "MERGE (n1: " + n1_info + ") ";
		std::string n2_cmd = "MERGE (n2: " + n2_info + ") ";

		// create edge
		std::string rel_str = EdgeEnum2String(edge->relation);
		std::string seq_str = uint128tostring(edge->seq);
		std::string timestamp = edge->timestamp;
		std::string e_cmd = "CREATE(n1)-[r:"+rel_str+"{seq:"+seq_str+", timestamp:\""+timestamp+"\"}]->(n2)";
		cmd = n1_cmd + n2_cmd + e_cmd;
		run(cmd);
	}
}

std::string Neo4jdb::EdgeCmd(KGEdge *edge, event_t e_seq) {
	std::string n1_hash = std::to_string(edge->n1_hash);
	std::string n2_hash = std::to_string(edge->n2_hash);	
		
	// node information
	std::string n1_str = "n" + std::to_string(e_seq * 2);
	std::string n2_str = "n" + std::to_string(e_seq * 2 + 1);
	std::string n1_cmd = "match (" + n1_str + " {hash:\"" + n1_hash + "\"}) ";
	std::string n2_cmd = "match (" + n2_str + " {hash:\"" + n2_hash + "\"}) ";

	// edge information
	std::string rel_str = EdgeEnum2String(edge->relation);
	std::string seq_str = uint128tostring(edge->seq);
	std::string r_str = "r" + std::to_string(e_seq);
	std::string timestamp = edge->timestamp;

	// create edge
	std::string e_cmd = "CREATE("+n1_str+")-["+r_str+":"+rel_str+"{seq:"+seq_str+", timestamp:\""+timestamp+"\"}]->("+n2_str+") with "+r_str+" ";

	std::string cmd = n1_cmd + n2_cmd + e_cmd;
	return cmd;
}

void Neo4jdb::Neo4jPrint() {
	neo4j_result_stream_t *results;
	neo4j_result_t *result;
	neo4j_value_t value;

	// Command for Neo4j Cypher
	char *run_cmd;
	int status = asprintf(&run_cmd, "match (n)-[r]-(m) return n,r,m");

	if (status == -1) {
		std::cerr << "buf copy fail" << std::endl;
	}
	
	// Receive results from Neo4j
	results = neo4j_run(connection, run_cmd, neo4j_null);	
	int ncolumns = neo4j_nfields(results);
	
	if (ncolumns < 0) {
		neo4j_perror(stderr, errno, "Failed to retrieve results");
	}

	// Parse and print result
	while ((result = neo4j_fetch_next(results)) != NULL) {
		for (int i = 0; i < ncolumns; i++) {
			value = neo4j_result_field(result, 0);
			neo4j_fprint(value, stdout);
		}
		std::cout << "\n";
	}

	free(run_cmd);
	neo4j_close_results(results);
}
