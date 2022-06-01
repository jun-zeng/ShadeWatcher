#include "postgresql.h"

Postgresqldb::Postgresqldb(std::string config_file) {
	std::map<std::string, std::string> cfg = LoadPostgresConfig(config_file);
	std::string username = cfg["username"];
	std::string host = cfg["host"];
	std::string password = cfg["password"];
	std::string dbname = cfg["dbname"];
	std::string port = cfg["port"];
	batch_node = std::stoi(cfg["batch_node"]);
	batch_edge = std::stoi(cfg["batch_edge"]);

	std::string config = "user=" + username +
						 " host=" + host +
						 " password=" + password +
						 " dbname=" + dbname;
						 " port=" + port;
	Connectdb(config);
}

Postgresqldb::~Postgresqldb() {
	// conn->disconnect();
	delete conn;
}

// Connect to database
void Postgresqldb::Connectdb(std::string config) {
	conn = new pqxx::connection(config);

	if (conn->is_open()) {
		std::cerr << "Database Connection successful!" << std::endl;
	}
	else {
		std::cerr << "Database Connection unsuccessful!" << std::endl;
	}
}

// Run SQL query
pqxx::result Postgresqldb::Run(std::string query) {
	pqxx::work w(*conn);

	pqxx::result res = w.exec(query);
	w.commit();
	return res;
}

void Postgresqldb::CreateSchema() {
	// create schema
	std::string sql;
	sql = "CREATE SCHEMA IF NOT EXISTS " + schema + ";";
	Run(sql);
}

void Postgresqldb::CreateNodeTable() {
	std::string sql;

	// create process table
	sql = "CREATE TABLE IF NOT EXISTS " + schema + ".proc ("	\
	"n_id		bigint		not null,"							\
	"pid		int			not null,"							\
	"exe 		text		not null,"							\
	"ppid 		int 		not null,"							\
	"args		text,"											\
	"PRIMARY KEY(n_id));";
	Run(sql);

	// create file table
	sql = "CREATE TABLE IF NOT EXISTS " + schema + ".file ("	\
	"n_id		bigint		not null,"							\
	"name		text		not null,"							\
	"version 	text,											\
	primary key(n_id));";
	Run(sql);

	// create socket table
	sql = "CREATE TABLE IF NOT EXISTS " + schema + ".socket ("	\
	"n_id		bigint		not null,"							\
	"name		text		not null,"							\
	"primary key(n_id));";
	Run(sql);

	// create node table
	sql = "CREATE TABLE IF NOT EXISTS " + schema + ".node ("	\
	"n_id		bigint		not null,"							\
	"n_type		int			not null,"							\
	"primary key(n_id));";
	Run(sql);
}

void Postgresqldb::CreateEdgeTable(int _file_id=0) {
	std::string sql;
	std::string file_id = std::to_string(_file_id);

	// create edge table - could be used for KG training
	sql = "CREATE TABLE IF NOT EXISTS " + schema + ".edge_" + file_id + " ("\
	"e_id 		bigint 		not null,"										\
	"n1_hash	bigint		not null,"										\
	"n2_hash	bigint		not null,"										\
	"relation	int			not null,"										\
	"sequence	bigint		not null,"										\
	"session	int			not null,"										\
	"timestamp	text,"														\
	"primary key(e_id));";
	Run(sql);
}

void Postgresqldb::InsertProcSQL(std::string &node_info, std::string &node_typ) {
	std::string sql1, sql2;
	node_info.pop_back();
	sql1 = "INSERT INTO " + schema + ".proc (n_id, pid, exe, ppid, args)"	\
	"VALUES" + node_info +
	"ON CONFLICT(n_id) DO NOTHING;";

	node_typ.pop_back();
	sql2 = "INSERT INTO " + schema + ".node (n_id, n_type)"					\
	"VALUES" + node_typ +
	"ON CONFLICT(n_id) DO NOTHING;";

	try {
		Run(sql1);
		Run(sql2);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	node_info.clear();
	node_typ.clear();
}

void Postgresqldb::LoadProcSQL(pqxx::result &res) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".proc;";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::LoadProcSQL(pqxx::result &res, std::string p_hash) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".proc WHERE n_id IN (" + p_hash + ");";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::InsertFileSQL(std::string &node_info, std::string &node_typ) {
	std::string sql1, sql2;
	node_info.pop_back();
	sql1 = "INSERT INTO " + schema + ".file (n_id, name, version) "			\
	"VALUES" + node_info +
	"ON CONFLICT(n_id) DO NOTHING;";

	node_typ.pop_back();
	sql2 = "INSERT INTO " + schema + ".node (n_id, n_type) "					\
	"VALUES" + node_typ +
	"ON CONFLICT(n_id) DO NOTHING;";

	try {
		Run(sql1);
		Run(sql2);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	node_info.clear();
	node_typ.clear();
}

void Postgresqldb::LoadFileSQL(pqxx::result &res) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".file;";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::LoadFileSQL(pqxx::result &res, std::string f_hash) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".file WHERE n_id IN (" + f_hash + ");";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::InsertSocketSQL(std::string &node_info, std::string &node_typ) {
	std::string sql1, sql2;
	node_info.pop_back();
	sql1 = "INSERT INTO " + schema + ".socket (n_id, name)"					\
	"VALUES" + node_info +
	"ON CONFLICT(n_id) DO NOTHING;";

	node_typ.pop_back();
	sql2 = "INSERT INTO " + schema + ".node (n_id, n_type)"					\
	"VALUES" + node_typ +
	"ON CONFLICT(n_id) DO NOTHING;";

	try {
		Run(sql1);
		Run(sql2);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	node_info.clear();
	node_typ.clear();
}

void Postgresqldb::LoadSocketSQL(pqxx::result &res) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".socket;";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::LoadSocketSQL(pqxx::result &res, std::string s_hash) {
	std::string sql;
	sql = "SELECT * FROM " + schema + ".socket WHERE n_id IN (" + s_hash + ");";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::InsertEmptyNodeSQL(std::string &node_id) {
	std::string sql;
	node_id.pop_back();
	sql = "INSERT INTO " + schema + ".node (n_id)"							\
	"VALUES" + node_id +
	"ON CONFLICT(n_id) DO NOTHING;";
	try {
		Run(sql);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	node_id.clear();
}

void Postgresqldb::InsertEdgeSQL(std::string &edge_info, int _file_id) {
	std::string sql;
	std::string file_id = std::to_string(_file_id);

	edge_info.pop_back();
	sql = "INSERT INTO " + schema + ".edge_" + file_id + " " 					\
	"(e_id, n1_hash, n2_hash, relation, sequence, session, timestamp)"		\
	"VALUES" + edge_info + 													
	"ON CONFLICT(e_id) DO NOTHING "												\
	"RETURNING e_id";
	try {
		Run(sql);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	edge_info.clear();
}

void Postgresqldb::LoadEdgeSQL(pqxx::result &res, int _file_id) {
	std::string sql;
	std::string file_id = std::to_string(_file_id);
	sql = "SELECT * FROM " + schema + ".edge_" + file_id + ";";
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void Postgresqldb::InsertKGNode(KG *infotbl) {
	// insert proc and node table
	int count = 0;
	std::string node_info, node_id, node_type;
	for (const auto &proc: infotbl->ProcNodeTable) {
		NodeProc *p = proc.second;

		hash_t _id = *(p->id);
		std::string id = std::to_string(_id);
		std::string pid = p->pid;
		std::string exe = p->exe;
		std::string ppid = p->ppid;
		std::string args = p->args;

		node_info += "(" + id + "," + pid + ",'" +
					 exe + "'," + ppid + ",'" +
					 args + "'),";
		node_id += "(" + id + "),";
		node_type += "(" + id + ", 1),";

		if (count != batch_node) {
			count++;
		}
		else {
			InsertProcSQL(node_info, node_type);
			// TODO:
			// InsertEmptyNodeSQL(node_id);
			count = 0;
		}
	}

	// In case that #node cannot be divided by #batch
	// If #node can be divided by #batch, count == 0
	if (count) {
		InsertProcSQL(node_info, node_type);
		// InsertEmptyNodeSQL(node_id);
		count = 0;
	}

	// insert file and node table
	for (const auto &file: infotbl->FileNodeTable) {
		NodeFile *f = file.second;

		hash_t _id = *(f->id);
		std::string id = std::to_string(_id);
		std::string name = f->name;
		std::string version = f->version;

		node_info += "(" + id + ",'" + name + "','" +
					 version + "'),";
		node_id += "(" + id + "),";
		node_type += "(" + id + ", 2),";

		if (count != batch_node) {
			count++;
		}
		else {
			InsertFileSQL(node_info, node_type);
			// InsertEmptyNodeSQL(node_id);
			count = 0;
		}
	}

	// In case that #node cannot be divided by #batch
	// If #node can be divided by #batch, count == 0
	if (count) {
		InsertFileSQL(node_info, node_type);
		// InsertEmptyNodeSQL(node_id);
		count = 0;
	}

	// insert socket and node table
	for (const auto &socket: infotbl->SocketNodeTable) {
		NodeSocket *s = socket.second;

		hash_t _id = *(s->id);
		std::string id = std::to_string(_id);
		std::string name = s->name;

		node_info += "(" + id + ",'" + name + "'),";
		node_id += "(" + id + "),";
		node_type += "(" + id + ", 3),";

		if (count != batch_node) {
			count++;
		}
		else {
			InsertSocketSQL(node_info, node_type);
			// InsertEmptyNodeSQL(node_id);
			count = 0;
		}
	}

	// In case that #node cannot be divided by #batch
	// If #node can be divided by #batch, count == 0
	if (count) {
		InsertSocketSQL(node_info, node_type);
		// InsertEmptyNodeSQL(node_id);
	}
}

void Postgresqldb::InsertOutNodeSQL(std::string &outnode_info,
								   std::set<std::string> &outnode_str) {
	std::string to_node;
	for (const auto &node: outnode_str) {
		to_node += node + ",";
	}
	to_node.pop_back();

	std::string sql;
	// update outnode in node table
	sql = "UPDATE node "								\
	"SET outnode = "										\
	"(CASE n_id " + outnode_info + " END)"				\
	"WHERE n_id in (" + to_node + ");";
	try {
		Run(sql);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	outnode_info.clear();
	outnode_str.clear();
}

void Postgresqldb::InsertInNodeSQL(std::string &innode_info,
								 	 std::set<std::string> &innode_str) {
	std::string from_node;
	for (const auto &node: innode_str) {
		from_node += node + ",";
	}
	from_node.pop_back();

	std::string sql;
	// update innode in node table
	sql = "UPDATE node "								\
	"SET innode = "									\
	"(CASE n_id " + innode_info + " END)"				\
	"WHERE n_id in (" + from_node + ");";
	try {
		Run(sql);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	innode_info.clear();
	innode_str.clear();
}

void Postgresqldb::InsertKGEdge(KG *infotbl, int file_id) {
	int count = 0;
	std::string edge_info;

	for (const auto it: infotbl->KGEdgeTable) {
		KGEdge* edge = it.second;
		// insert edges
		hash_t _e_id = edge->e_id;
		hash_t _n1_hash = edge->n1_hash;
		hash_t _n2_hash = edge->n2_hash;
		std::string e_id = std::to_string(_e_id);
		std::string n1_hash = std::to_string(_n1_hash);
		std::string n2_hash = std::to_string(_n2_hash);
		std::string relation = std::to_string(EdgeEnum2Int(edge->relation));
		std::string sequence = uint128tostring(edge->seq);
		std::string session = std::to_string(edge->sess);
		std::string ts = edge->timestamp;
		
		std::string temp_edge_info = n1_hash + "," + n2_hash + "," + relation + "," 
					 				 + sequence + "," + session + "," + "'" + ts + "'";
		edge_info += "(" + e_id + "," + temp_edge_info + "),";

		count++;
		if (count == batch_edge) {
			InsertEdgeSQL(edge_info, file_id);
			count = 0;
			// break;
		}
	}

	if (count) {
		InsertEdgeSQL(edge_info, file_id);
	}
}

void Postgresqldb::PrintRes(pqxx::result res) {
	for (auto row: res) {	 
		for (auto field: row) 
			std::cout << field.c_str() << '\t';
		std::cout << std::endl;
	}
}

int Postgresqldb::CountTblNum() {
	std::string sql;
	sql = "SELECT count(*) "						\
	"FROM information_schema.tables "				\
	"WHERE table_schema = '" + schema + "';";		\
	pqxx::result res;
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	for (pqxx::result::const_iterator it = res.begin(); it != res.end(); it++) {
		int tblnum = it[0].as<int>();
		return tblnum;
	}
	std::cout << "Cannot retrieve the table number in schema: " + 
			schema + ". Directly exit!" << std::endl;  
	exit(0);
}

void KGLoadAllFromDB(std::string postgres_config, KG *infotbl, std::string _schema) {
	auto start = OverheadStart();
	std::cout << "Load system entity and edges facts from database" << std::endl;

	Postgresqldb db(postgres_config);
	db.schema = _schema;

	// read facts
	pqxx::result res_p;
	db.LoadProcSQL(res_p);
	pqxx::result res_f;
	db.LoadFileSQL(res_f);
	pqxx::result res_s;
	db.LoadSocketSQL(res_s);

	// We use another function to support darpa dataset partial loading:
	// e.g., there should be two modes for Darpa: Load-All mode and Load Partial mode 
	// Load-All is the same as the current
	// Load-Partial should support loading one darpa file first: load edges in one file,
	// based on the edges, figure out which nodes it needs => searching in proc/file/sock tbl. 
	std::cout << "\tLoad process entity from database" << std::endl;
	for (pqxx::result::const_iterator it = res_p.begin(); it != res_p.end(); it++) {
		hash_t p_hash = it[0].as<hash_t>();
		std::string pid = it[1].as<std::string>();
		std::string exe = it[2].as<std::string>();
		std::string ppid = it[3].as<std::string>();
		std::string args = it[4].as<std::string>();

		NodeProc *p_temp = new NodeProc (pid, exe, args, ppid, p_hash);
		infotbl->InsertProc(p_temp);
		infotbl->InsertNode(p_hash, NodeType_t::Proc);
	}

	std::cout << "\tLoad file entity from database" << std::endl;
	for (pqxx::result::const_iterator it = res_f.begin(); it != res_f.end(); it++) {
		hash_t f_hash = it[0].as<hash_t>();
		std::string f_name = it[1].as<std::string>();
		std::string f_version = it[2].as<std::string>();

		NodeFile *f_temp = new NodeFile (f_name, f_version, f_hash);
		infotbl->InsertFile(f_temp);
		infotbl->InsertNode(f_hash, NodeType_t::File);
	}

	std::cout << "\tLoad socket entity from database" << std::endl;
	for (pqxx::result::const_iterator it = res_s.begin(); it != res_s.end(); it++) {
		hash_t s_hash = it[0].as<hash_t>();
		std::string s_name = it[1].as<std::string>();
		
		NodeSocket *s_temp = new NodeSocket (s_name, s_hash);
		infotbl->InsertSocket(s_temp);
		infotbl->InsertNode(s_hash, NodeType_t::Socket);
	}

	// load edges
	int edge_file_num;
	db.total_file_num = db.CountTblNum();
	edge_file_num = db.total_file_num - db.non_edg_tbl_num;
	for (int i = 0; i < edge_file_num; i++) {
		pqxx::result res_e;
		db.LoadEdgeSQL(res_e, i);

		std::cout << "\tLoad edge_" << i << " from database" << std::endl;
		for (pqxx::result::const_iterator it = res_e.begin(); it != res_e.end(); it++) {
			hash_t e_hash = it[0].as<hash_t>();
			hash_t n1_hash = it[1].as<hash_t>();
			hash_t n2_hash = it[2].as<hash_t>();
			EdgeType_t rel = (EdgeType_t) it[3].as<int>();

			// postgresql supports bigint which is 8 bytes, but seq_t is uint128_t
			seq_t seq = stoint128_t(it[4].as<std::string>());
			sess_t sess = it[5].as<sess_t>();
			std::string ts = it[6].as<std::string>();

			KGEdge *e_temp = new KGEdge (n1_hash, n2_hash, rel, seq, sess, e_hash, ts);
			infotbl->InsertEdge(e_temp);
		}
	}

	infotbl->GenKGEdgeTableWhenLoad();
	infotbl->edge_num += infotbl->KGEdgeTable.size();

	OverheadEnd(start, "System entities and edges Loading");
}

// Check the type of a node when doing partial loading edges in DARPA 
// 1 is proc, 2 is file and 3 is socket 
int Postgresqldb::CheckNodeTyp(std::string &n_hash) {
	std::string sql;
	// TODO: Could we achieve batch check and load for node type?
	sql = "SELECT n_type FROM " + schema + ".node WHERE n_id IN (" + n_hash + ");";
	pqxx::result res;
	try {
		pqxx::result R(Run(sql));
		res = R;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	for (pqxx::result::const_iterator it = res.begin(); it != res.end(); it++) {
		int typ = it[0].as<int>();
		return typ;
	}
	std::cout << "Cannot retrieve the node type in schema: " + 
			schema + ".node table. Hash_id: " + n_hash + " Directly exit!" << std::endl;  
	exit(0);
}

void KGLoadPartialFromDB(std::string postgres_config, KG *infotbl, std::string _schema, std::string edg_ld_files) {
	auto start = OverheadStart();
	std::cout << "Load system entity and edges facts partially (" << edg_ld_files << ") from database" << std::endl;

	Postgresqldb db(postgres_config);
	db.schema = _schema;

	std::vector<std::string> dar_file_vec = split(edg_ld_files, ',');
	std::set<std::string> node_set;
	for (auto vec: dar_file_vec) {
		int i = std::stoi(vec);
		pqxx::result res_e;
		db.LoadEdgeSQL(res_e, i);

		// query proc, file and socket based on the edge processing
		std::cout << "\tLoad edges from database" << std::endl;
		for (pqxx::result::const_iterator it = res_e.begin(); it != res_e.end(); it++) {
			hash_t e_hash = it[0].as<hash_t>();

			// deal with the n1 node
			hash_t n1_hash = it[1].as<hash_t>();
			std::string n1_hash_str = it[1].as<std::string>();
			node_set.insert(n1_hash_str);
			
			// deal with the n2 node
			hash_t n2_hash = it[2].as<hash_t>();
			std::string n2_hash_str = it[2].as<std::string>();
			node_set.insert(n2_hash_str);

			EdgeType_t rel = (EdgeType_t) it[3].as<int>();

			// postgresql supports bigint which is 8 bytes, but seq_t is uint128_t
			seq_t seq = stoint128_t(it[4].as<std::string>());
			sess_t sess = it[5].as<sess_t>();
			std::string ts = it[6].as<std::string>();

			KGEdge *e_temp = new KGEdge (n1_hash, n2_hash, rel, seq, sess, e_hash, ts);
			infotbl->InsertEdge(e_temp);
		}
	}

	for (auto n: node_set) {
		int typ = db.CheckNodeTyp(n);
		if (typ == 1) {
			// search this node in proc tbl
			pqxx::result res_p;
			db.LoadProcSQL(res_p, n);
			for (pqxx::result::const_iterator it = res_p.begin(); it != res_p.end(); it++) {
				hash_t p_hash = it[0].as<hash_t>();
				std::string pid = it[1].as<std::string>();
				std::string exe = it[2].as<std::string>();
				std::string ppid = it[3].as<std::string>();
				std::string args = it[4].as<std::string>();

				NodeProc *p_temp = new NodeProc (pid, exe, args, ppid, p_hash);
				infotbl->InsertProc(p_temp);
				infotbl->InsertNode(p_hash, NodeType_t::Proc);
			}
		} else if (typ == 2) {
			// search this node in file tbl
			pqxx::result res_f;
			db.LoadFileSQL(res_f, n);
			for (pqxx::result::const_iterator it = res_f.begin(); it != res_f.end(); it++) {
				hash_t f_hash = it[0].as<hash_t>();
				std::string f_name = it[1].as<std::string>();
				std::string f_version = it[2].as<std::string>();

				NodeFile *f_temp = new NodeFile (f_name, f_version, f_hash);
				infotbl->InsertFile(f_temp);
				infotbl->InsertNode(f_hash, NodeType_t::File);
			}
		} else if (typ == 3) {
			// search this node in socket tbl
			pqxx::result res_s;
			db.LoadSocketSQL(res_s, n);
			for (pqxx::result::const_iterator it = res_s.begin(); it != res_s.end(); it++) {
				hash_t s_hash = it[0].as<hash_t>();
				std::string s_name = it[1].as<std::string>();
				
				NodeSocket *s_temp = new NodeSocket (s_name, s_hash);
				infotbl->InsertSocket(s_temp);
				infotbl->InsertNode(s_hash, NodeType_t::Socket);
			}	
		}
	}

	infotbl->GenKGEdgeTableWhenLoad();

	OverheadEnd(start, "System entities and edges partial Loading");
}

void KGStoreToDB(std::string postgres_config, KG *infotbl, std::string _schema, int file_id) {
	auto dbstart = OverheadStart();
	std::cout << "Connected to postgres. Prepare to store KG into database.." << std::endl;
	Postgresqldb db(postgres_config);
	db.schema = _schema;
	if (file_id == 0) {
		db.CreateSchema();
		db.CreateNodeTable();
	}
	db.CreateEdgeTable(file_id);
	db.InsertKGNode(infotbl);
	db.InsertKGEdge(infotbl, file_id);	
	
	OverheadEnd(dbstart, "Store KG to DB");
}

void KGStoreNodeToDB(std::string postgres_config, KG *infotbl, std::string _schema, int file_id) {
	auto dbstart = OverheadStart();
	std::cout << "Connected to postgres. Prepare to store KG nodes into database.." << std::endl;
	Postgresqldb db(postgres_config);
	db.schema = _schema;
	if (file_id == 0) {
		db.CreateSchema();
	}
	db.CreateNodeTable();
	db.InsertKGNode(infotbl);
	
	OverheadEnd(dbstart, "Store KG Node to DB");
}

void KGStoreEdgeToDB(std::string postgres_config, KG *infotbl, std::string _schema, int file_id) {
	auto dbstart = OverheadStart();
	std::cout << "Connected to postgres. Prepare to store graph into database.." << std::endl;
	Postgresqldb db(postgres_config);
	db.schema = _schema;
	if (file_id == 0) {
		db.CreateSchema();
	}
	db.CreateEdgeTable(file_id);
	db.InsertKGEdge(infotbl, file_id);	
	
	OverheadEnd(dbstart, "Store KG to DB");
}

void KGDeleteSchema(std::string postgres_config, std::string _schema) {
	std::cout << "Going to delete schema " + _schema + ".." << std::endl;
	Postgresqldb db(postgres_config);
	std::string sql = "DROP SCHEMA " + _schema + " CASCADE;";
	db.Run(sql);
	std::cout << "Delete done. Please check!" << std::endl;
}
