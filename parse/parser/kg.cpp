#include "kg.h"

KG::KG(int _audit_source) {
	audit_source = _audit_source;

	event_num = 0;
	edge_num = 0;

	// List procs and files that may trigger dependency explosion problem
	std::string noise_proc[] = {"firefox", "command-not-fou"};
	std::string noise_file[] = {".so.", "/proc", "/dev", ".bash_history", "bash_completion.d","/run/motd.dynamic", "/bin/sh",".bash_logout"};

	for (auto proc: noise_proc) {
		NoiseProc.push_back(proc);
	}

	for (auto file: noise_file) {
		NoiseFile.push_back(file);
	}
}

KG::~KG() {
	ProcNodeTable.clear();
	FileNodeTable.clear();
	SocketNodeTable.clear();
	AttrNodeTable.clear();
	ProcFdMap.clear();
	PidTable.clear();
	KGEdgeTable.clear();
	KGNodeTable.clear();
	NoiseTable.clear();
	FileInteractionTable.clear();
	ProcInteractionTable.clear();
}

void KG::FreeNode() {
	for (auto it: ProcNodeTable) {
		delete (it.second);
	}

	for (auto it: FileNodeTable) {
		delete (it.second);
	}

	for (auto it: SocketNodeTable) {
		delete (it.second);
	}

	for (auto it: AttrNodeTable) {
		delete (it.second);
	}

	for (auto it: ProcFdMap) {
		delete (it.second);
	}

	ProcNodeTable.clear();
	FileNodeTable.clear();
	SocketNodeTable.clear();
	AttrNodeTable.clear();
	ProcFdMap.clear();
	PidTable.clear();
	KGNodeTable.clear();
	NoiseTable.clear();
}

void KG::FreeInteraction() {
	for (auto it: KGEdgeTable) {
		delete (it.second);
	}
	
	for (auto it: FileInteractionTable) {
		std::vector<KGEdge *>().swap(*it.second);
		delete(it.second);
	}

	for (auto it: ProcInteractionTable) {
		std::vector<KGEdge *>().swap(*it.second);
		delete(it.second);
	}

	KGEdgeTable.clear();
	FileInteractionTable.clear();
	ProcInteractionTable.clear();
}

NodeProc* KG::SearchProc(hash_t _id) {
	auto it = ProcNodeTable.find(_id);
	if (it != ProcNodeTable.end())
		return it->second;
	else {
		// std::cerr << "Cannot find this Proc" << _id << std::endl;
		return NULL;
	}
}

NodeProc* KG::SearchProc(std::string _pid) {
	auto it = PidTable.find(_pid);
	if (it != PidTable.end())
		return it->second;
	else {
		// std::cerr << "Cannot find this Proc" << _pid << std::endl;
		return NULL;
	}
}

NodeFile* KG::SearchFile(hash_t _id) {
	auto it = FileNodeTable.find(_id);
	if (it != FileNodeTable.end())
		return it->second;
	else {
		// std::cerr << "Cannot find this File when doing SearchFile" << _id << std::endl;
		return NULL;
	}
}

NodeSocket* KG::SearchSocket(hash_t _id) {
	auto it = SocketNodeTable.find(_id);
	if (it != SocketNodeTable.end())
		return it->second;
	else {
		// std::cerr << "Cannot find this Socket" << _id << std::endl;
		return NULL;
	}
}

NodeAttr* KG::SearchAttr(hash_t _id) {
	auto it = AttrNodeTable.find(_id);
	if (it != AttrNodeTable.end())
		return it->second;
	else {
		// std::cerr << "Cannot find this Attribute" << _id << std::endl;
		return NULL;
	}
}

void KG::InsertNode(hash_t _id, NodeType_t type) {
	KGNodeTable[_id] = type;
}

void KG::InsertNoisyNode(hash_t _id, NodeType_t type) {
	NoiseTable[_id] = type;
}

void KG::InsertPid(std::string _id, NodeProc *p) {
	PidTable[_id] = p;
}

NodeProc* KG::InsertProc(NodeProc *p) {
	hash_t p_hash = *(p->id);
	auto it = ProcNodeTable.find(p_hash);
	if (it != ProcNodeTable.end()) {
		delete (p);
		return it->second;
	}

	ProcNodeTable[p_hash] = p;
	InsertNode(p_hash, p->type);

	// Noisy nodes are used as termination condition in Tracker
	std::string exe = p->exe;
	for (std::string proc: NoiseProc) {
		if (exe.find(proc) != std::string::npos) {
			InsertNoisyNode(p_hash, NodeType_t::Proc);
			break;
		}
	}
	
	return p;
}

bool KG::InsertLoginProc(const Json::Value event, LogLoader *logload, event_t id) {
	std::string pid = Jval2str(event["process"]["pid"]); 
	std::string exe = Jval2str(event["process"]["exe"]); 
	NodeProc *p = SearchProc(pid);

	if (p) {
		// login proc has been executed; no need to create a new proc
		if (p->exe.compare(" ") != 0)
			return false;

		// process has been created
		hash_t p_id_old = *(p->id);
		// Becasue args is only available in execute system call,
		// we set args as " " by default for login process
		p->UpdateID(exe, " ");
		hash_t p_new_id = *(p->id);
		// update ProcNodeTable, KGNodeTable, ProcFdMap, and PidTable
		ExchangeMapKey(ProcNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(KGNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(ProcFdMap, p_id_old, p_new_id);
		ExchangeMapKey(ProcInteractionTable, p_id_old, p_new_id);
	}
	else {
		// process does not exist
		NodeProc *p_login = new NodeProc (pid, exe);
		p_login = InsertProc(p_login);
		InsertPid(pid, p_login);
		
		std::string ppid = "-1";
		// we hardcode the number of events to search (parent of pid) as 500 
		// should be logload->event_num - 1
		Json::Value event_search;
		for (event_t id_tmp = id; id_tmp < (id + 500); id_tmp++) {
			logload->GetEvent(id_tmp, event_search);
			if (event_search["auditd"]["data"].isMember("syscall")) {
				std::string pid_search = Jval2str(event_search["process"]["pid"]);
				if (pid.compare(pid_search) == 0) {
					ppid = Jval2str(event_search["process"]["ppid"]);
					break;
				}
			}
		}

		NodeProc *pp = SearchProc(ppid);
		hash_t p_login_id = *(p_login->id);
		if (pp) {
			p_login->ppid = ppid;
			// copy parent's fd to child
			hash_t pp_id = *(pp->id);
			CopyFd(pp_id, p_login_id);
		}
		else {
			std::vector<hash_t> *fd_vec = new std::vector<hash_t> (0);
			ProcFdMap[p_login_id] = fd_vec;
			// std::cerr << "loginproc cannot find parent for proc " << pid << std::endl;
			// return false;
		}
	}
	return true;
}

// Insert LoginProc for driverkafka
bool KG::InsertLoginProc(const Json::Value event, std::map<seq_t, Json::Value> msgs, std::map<seq_t, Json::Value>::iterator msg) {
	std::string pid = Jval2str(event["process"]["pid"]); 
	std::string exe = Jval2str(event["process"]["exe"]); 
	NodeProc *p = SearchProc(pid);

	if (p) {
		// login proc has been executed; no need to create a new proc
		if (p->exe.compare(" ") != 0)
			return false;

		// process has been created
		hash_t p_id_old = *(p->id);
		// Becasue args is only available in execute system call,
		// we set args as " " by default for login process
		p->UpdateID(exe, " ");
		hash_t p_new_id = *(p->id);
		// update ProcNodeTable, KGNodeTable, ProcFdMap, and PidTable
		ExchangeMapKey(ProcNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(KGNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(ProcFdMap, p_id_old, p_new_id);
		ExchangeMapKey(ProcInteractionTable, p_id_old, p_new_id);
	}
	else {
		// process does not exist
		NodeProc *p_login = new NodeProc (pid, exe);
		p_login = InsertProc(p_login);
		InsertPid(pid, p_login);
		
		std::string ppid = "-1";
		// we hardcode the number of events to search (parent of pid) as 500 
		Json::Value event_search;
		for (; msg != msgs.end(); msg++) {
			Json::Value event_search = msg->second;
			if (event_search["auditd"]["data"].isMember("syscall")) {
				std::string pid_search = Jval2str(event_search["process"]["pid"]);
				if (pid.compare(pid_search) == 0) {
					ppid = Jval2str(event_search["process"]["ppid"]);
					break;
				}
			}
		}

		NodeProc *pp = SearchProc(ppid);
		hash_t p_login_id = *(p_login->id);
		if (pp) {
			p_login->ppid = ppid;
			// copy parent's fd to child
			hash_t pp_id = *(pp->id);
			CopyFd(pp_id, p_login_id);
		}
		else {
			std::vector<hash_t> *fd_vec = new std::vector<hash_t> (0);
			ProcFdMap[p_login_id] = fd_vec;
			// std::cerr << "loginproc cannot find parent for proc " << pid << std::endl;
			// return false;
		}
	}
	return true;
}

void KG::PrintNodeTable() {
	for (const auto &node: KGNodeTable) {
		std::cout << node.first << " " << std::endl;
	}
}

void KG::PrintProcTable(){
	for (const auto &node : ProcNodeTable){
		std::cout << node.first << "\tpid " << node.second->pid 
				  << "\texe " << node.second->exe 
				  << "\tppid " << node.second->ppid
				  << "\targs " << node.second->args << std::endl;
	}
}

void KG::PrintPidTable(){
	for (const auto &node: PidTable){
		std::cout << "pid " << node.second->pid 
				  << "\texe " << node.second->exe 
				  << "\tppid " << node.second->ppid
				  << "\targs " << node.second->args << std::endl;
	}
}

NodeFile* KG::InsertFile(NodeFile *f) {
	hash_t f_hash = *(f->id);
	auto it = FileNodeTable.find(f_hash);
	if (it != FileNodeTable.end()) {
		delete (f);
		return it->second;
	}

	FileNodeTable[f_hash] = f;
	InsertNode(f_hash, f->type);

	// Noisy nodes are used as termination condition in Tracker
	std::string name = f->name;
	for (std::string file: NoiseFile) {
		if (name.find(file) != std::string::npos) {
			InsertNoisyNode(f_hash, NodeType_t::File);
			break;
		}
	}

	return f;
}

void KG::PrintFileTable() {
	for (const auto &node: FileNodeTable) {
		std::cout << node.first 
		<< "\tname " << node.second->name 
		<< "\tversion " << node.second->version << std::endl;
	}
}

NodeSocket* KG::InsertSocket(NodeSocket *s) {
	hash_t s_id = *(s->id);
	auto it = SocketNodeTable.find(s_id);
	if (it != SocketNodeTable.end()) {
		delete (s);
		return it->second;
	}
	else {
		SocketNodeTable[s_id] = s;
		// SocketNodeTable.insert(std::pair<hash_t , NodeSocket *>(*(socket->id), socket));
		InsertNode(s_id, s->type);
		return s;
	}
}

void KG::PrintSocketTable() {
	for (const auto &node: SocketNodeTable) {
		std::cout << node.first 
		<< "\tname "<< node.second->name << std::endl;
	}
}

NodeAttr* KG::InsertAttr(NodeAttr *attr) {
	auto it = AttrNodeTable.find(*(attr->id));
	if (it != AttrNodeTable.end()) {
		delete (attr);
		return it->second;
	}
	else {
		AttrNodeTable[*(attr->id)] = attr;
		// AttrNodeTable.insert(std::pair<hash_t , NodeAttr *>(*(attr->id), attr));
		hash_t attrid = *(attr->id);
		InsertNode(attrid, attr->type);
		return attr;
	}
}

void KG::PrintAttrTable() {
	for (const auto &node: AttrNodeTable){
		std::cout << node.first 
				  << "\tattrtype " << node.second->attrtype
				  << "\tvalue " << node.second->value << std::endl;
	} 
}

void KG::InsertEdge(KGEdge *edge) {
	KGEdgeList.push_back(edge);
}

void KG::InsertEdge(hash_t e_id, KGEdge *edge) {
	// Todo: address hash collision problem
	while (1) {
		auto _it = KGEdgeTable.find(e_id);
		if (_it != KGEdgeTable.end()) {
			e_id++;
			edge->e_id = e_id;
		}
		else {
			break;
		}
	}

    KGEdgeTable[e_id] = edge;
}

NodeType_t KG::SearchNodeType(hash_t _id) {
	auto it = KGNodeTable.find(_id);
	if (it == KGNodeTable.end()){
		// std::cerr << "miss type" + std::to_string(_id);
		return NodeType_t::NotDefined;
	}
	return it->second;
}

bool KG::SearchNodeNotExist(hash_t _id) {
	auto it = KGNodeTable.find(_id);
	if (it == KGNodeTable.end()){
		return true;
	}
	else {
		return false;
	}
}

std::pair<NodeType_t, std::string> KG::SearchNode(hash_t _id) {
	auto it = KGNodeTable.find(_id);
	if (it == KGNodeTable.end()){
		std::cerr << "miss node" + std::to_string(_id);
		return std::make_pair(NodeType_t::NotDefined, "miss when searching");
	}

	std::string info;
	switch (it->second){
		case NodeType_t::Proc:{
			NodeProc *p = SearchProc(_id);
			std::string p_id = std::to_string(*(p->id));
			info = p->pid + " " + p->exe + " " + p->args + " " + p_id;
			info.erase(std::remove(info.begin(), info.end(), '\"'), info.end());
			break;
		}
		case NodeType_t::File:{
			NodeFile *f = SearchFile(_id);
			std::string f_id = std::to_string(*(f->id));
			info = f->name + " " + " " + f_id;
			break;
		}
		case NodeType_t::Socket:{
			NodeSocket *s = SearchSocket(_id);
			std::string s_id = std::to_string(*(s->id));
			info = s->name + " " + s_id;
			break;
		}
		case NodeType_t::Attr:{
			NodeAttr *a = SearchAttr(_id);
			std::string a_id = std::to_string(*(a->id));
			info = a->attrtype + " " + a->value + " " + a_id;
			break;
		}
		case NodeType_t::NotDefined:{
			break;
		}
	}
	std::pair<NodeType_t, std::string> ret;
	ret = std::make_pair(it->second, info);

	return ret;
}

void KG::PrintEdgeTable() {
	for (auto &it: KGEdgeTable){
		KGEdge *edge = it.second;
		std::string n1 = SearchNode(edge->n1_hash).second;
		std::string n2 = SearchNode(edge->n2_hash).second;
		std::string e_id_str = std::to_string(edge->e_id);

		std::cout << "e_id " << e_id_str;
		std::cout << "sess " << edge->sess << "\tseq " << uint128tostring(edge->seq);

		std::cout << "\n" << n1;
		std::cout << "\n" << EdgeEnum2String(edge->relation);
		std::cout << "\n" << n2 << "\n" << std::endl;
	}
}

void KG::PrintEdges(std::vector<KGEdge *> edges) {
	for (const auto edge: edges){
		std::string n1 = SearchNode(edge->n1_hash).second;
		std::string n2 = SearchNode(edge->n2_hash).second;

		std::cout << "sess " << edge->sess << "\tseq " << uint128tostring(edge->seq);

		std::cout << "\n" << n1;
		std::cout << "\n" << EdgeEnum2String(edge->relation);
		std::cout << "\n" << n2 << "\n" << std::endl;
	}
}

void KG::PrintKG(){
	auto proc_num = ProcNodeTable.size();
	auto file_num = FileNodeTable.size();
	auto socket_num = SocketNodeTable.size();
	auto node_num = KGNodeTable.size();

	std::cout << "KG Statistics" << std::endl;
	std::cout << "#Events: " << event_num << std::endl;
	std::cout << "#Edge: " << edge_num << std::endl;
	std::cout << "#Noisy events: " << noise_num << std::endl;
	std::cout << "#Proc: " << proc_num << std::endl;
	std::cout << "#File: " << file_num << std::endl;
	std::cout << "#Socket: " << socket_num << std::endl;
	std::cout << "#Node: " << node_num << std::endl;
	std::cout << "#Node(" << node_num << ") = #Proc(" << proc_num
	<< ") + #File(" << file_num << ") + #Socket(" << socket_num << ")\n" << std::endl;
}

bool KG::LoadSocket(std::string socket_path) {
	std::string generalPath = socket_path + "/general.txt";
	std::string devicePath = socket_path + "/device.txt";
	std::string namePath = socket_path + "/name.txt";

	std::ifstream generalfile(generalPath, std::ios::in);
	if (!generalfile.is_open()){
		std::cerr << "Fail to open file:" << generalPath << std::endl;
		return false; 
	}
	fd_t socket_num = std::count(
		std::istreambuf_iterator<char>(generalfile), 
		std::istreambuf_iterator<char>(), '\n') - 1;
	
	std::string line;
	std::ifstream devicefile(devicePath, std::ios::in);
	if (!devicefile.is_open()){
		std::cerr << "Fail to open file:" << devicePath << std::endl;
		return false; 
	}
	std::getline(devicefile, line);

	std::ifstream namefile(namePath, std::ios::in);
	if (!namefile.is_open()) {
		std::cerr << "Fail to open file:" << namePath << std::endl;
		return false;
	}
	std::getline(namefile, line);

	std::string device;
	std::string name;
	for (fd_t it = 0; it < socket_num; it++) {
		std::getline(devicefile, device);
		std::getline(namefile, name);

		NodeSocket *s = SearchSocket(device);
		if (s == NULL) {
			continue;
		}
		hash_t s_id_old = *(s->id);

		// Parse IP address and port for sockets
		std::string new_name;
		// find -> in socket name
		auto found_1 = name.find("->");
		// find * in socket name
		auto found_2 = name.find("*", found_1 + 1);
		// Todo: we consider [::] as localhost
		auto found_3 = name.find("]");

		if (found_2 != std::string::npos) {
			// find pos of : for port
			auto pos = name.find(":", found_1 + 1);
			new_name = "127.0.0.1" + name.substr(pos);
		}
		else {
			if (found_3 != std::string::npos) {
				// find pos of : for port
				auto pos = name.find(":", found_3 + 1);
				new_name = "127.0.0.1" + name.substr(pos);
			}
			else if (found_1 != std::string::npos) {
				new_name = name.substr(found_1 + 2);
			}
			else {
				new_name = name;
			}
		}
		
		// Update SocketNodeTable and KGNodeTable maps
		// (1) delete old socket named socket[xxxx]
		KGNodeTable.erase(s_id_old);
		auto nh_socket = SocketNodeTable.extract(s_id_old);
		s = nh_socket.mapped();
		delete(s);
		// (2) create a socket with new name
		NodeSocket *s_tmp = new NodeSocket (new_name);
		s = InsertSocket(s_tmp);
		hash_t s_id_new = *(s->id);
		
		// Previously we used extract, insert and move to update SocketNodemap
		// and KGNodemap, but they cause memory leakage
		/*
		hash_t s_id_old = *(s->id);
		s->UpdateID(new_name);
		hash_t s_id_new = *(s->id);
		ExchangeMapKey(SocketNodeTable, s_id_old, s_id_new);
		ExchangeMapKey(KGNodeTable, s_id_old, s_id_new);
		*/

		// update ProcFdMap
		for (auto procfd: ProcFdMap) {
			std::vector<hash_t> *fd_vec = procfd.second;
			std::vector<hash_t>::iterator it;
			it = find(fd_vec->begin(), fd_vec->end(), s_id_old);
			if (it != fd_vec->end()){
				*it = s_id_new;
			}
		}
	}

	generalfile.close();
	devicefile.close();
	namefile.close();

	return true;
}

NodeSocket* KG::SearchSocket(std::string _device) {
	for (auto it: SocketNodeTable) {
		std::string name = it.second->name;
		auto start = name.find("[") + 1;
		auto num = name.find("]") - start - 2;
		std::string device = name.substr(start, num);

		if (device == _device) {
			return it.second;
		}
	}
	return NULL;
}

bool KG::LoadProc(std::string procPath) {
	std::string generalPath = procPath + "/general.txt";
	std::string pidPath = procPath + "/pid.txt";
	std::string exePath = procPath + "/exe.txt";
	std::string argsPath = procPath + "/args.txt";
	std::string ppidPath = procPath + "/ppid.txt";

	// read metadata information
	std::string line;

	std::ifstream procfile(generalPath, std::ios::in);
	if (!procfile.is_open()){
		std::cerr << "Fail to open file:" << generalPath << std::endl;
		return false; 
	}

	std::ifstream pidfile(pidPath, std::ios::in);
	if (!pidfile.is_open()){
		std::cerr << "Fail to open file:" << pidPath << std::endl;
		return false; 
	}
	std::getline(pidfile, line);

	std::ifstream exefile(exePath, std::ios::in);
	if (!exefile.is_open()){
		std::cerr << "Fail to open file:" << exePath << std::endl;
		return false; 
	}
	std::getline(exefile, line);

	std::ifstream argsfile(argsPath, std::ios::in);
	if (!argsfile.is_open()){
		std::cerr << "Fail to open file:" << argsPath << std::endl;
		return false; 
	}
	std::getline(argsfile, line);

	std::ifstream ppidfile(ppidPath, std::ios::in);
	if (!ppidfile.is_open()){
		std::cerr << "Fail to open file:" << ppidPath << std::endl;
		return false; 
	}
	std::getline(ppidfile, line);

	proc_t procNum;
	procNum = std::count(std::istreambuf_iterator<char>(procfile), 
			   std::istreambuf_iterator<char>(), '\n') - 2;

	for (proc_t it = 0; it <= procNum; it++){
		std::string pid;
		std::string exe;
		std::string args;
		std::string ppid;

		std::getline(pidfile, pid);
		std::getline(exefile, exe);
		std::getline(argsfile, args);
		std::getline(ppidfile, ppid);

		std::istringstream ipid(pid); 
		ipid >> pid;

		NodeProc *p_temp = new NodeProc (pid, exe, args, ppid);
		NodeProc *p = InsertProc(p_temp);
		InsertPid(pid, p);
	}

	procfile.close();
	pidfile.close();
	exefile.close();
	argsfile.close();
	ppidfile.close();
	return true;
}

bool KG::LoadFd(std::string fd_path) {
	DIR *fddir;
	fddir = opendir(fd_path.c_str());
	if (fddir == NULL){
		std::cerr << "Fail to open file:" << fd_path << std::endl;
		return false; 
	}

	struct dirent * ptr;
	while((ptr = readdir(fddir)) != NULL) {
		if (ptr->d_name[0] == '.') {
			continue;
		}

		std::string fd_file_path = fd_path + "/" + ptr->d_name; 
		std::ifstream fdfile(fd_file_path, std::ios::in);
		if (!fdfile.is_open()) {
			std::cerr << "Fail to open file:" << fd_file_path << std::endl;
			continue;
		}
		if (fdfile.peek() == std::ifstream::traits_type::eof()) {
			// std::cerr << "Empty file:" << fd_file_path << std::endl;
			continue;
		}

		fd_t fd_num = std::count(std::istreambuf_iterator<char>(fdfile), 
			   std::istreambuf_iterator<char>(), '\n') - 3;
		fdfile.seekg (0, fdfile.beg);
		// std::cout << fd_num << std::endl;

		std::string pid = ptr->d_name;
		NodeProc *p = SearchProc(pid);
		// Cannot find process pid in ProcNodeMap
		if (!p) {
			continue;
		}

		std::vector<hash_t> *fd_vec = new std::vector<hash_t> (fd_num);
		std::string fd_line;
		// read total, ., and ..
		std::getline(fdfile, fd_line);
		std::getline(fdfile, fd_line);
		std::getline(fdfile, fd_line);

		while (std::getline(fdfile, fd_line)) {
			// example: 0 -> /dev/null
			size_t loc = fd_line.rfind("-> ");
			size_t loc_ = fd_line.rfind(" ", loc - 2);

			std::string f_name = fd_line.substr(loc + 3);
			fd_t fd = std::stoi(fd_line.substr(loc_ + 1, loc - loc_ - 1));
			fd_t fd_vec_size = fd_vec->size();
			if (fd >= fd_vec_size) {
				for (int i = 0; i <= int(fd - fd_vec_size); i++) {
					fd_vec->push_back(0);
				}
			}

			hash_t fd_id;
			if (f_name.find("socket") != std::string::npos) {
				NodeSocket *s_tmp = new NodeSocket (f_name);
				NodeSocket *s = InsertSocket(s_tmp);
				fd_id = *(s->id);
			}
			else {
				 // Todo: add i_version for file
				NodeFile *f_tmp = new NodeFile (f_name);
				NodeFile *f = InsertFile(f_tmp);
				fd_id = *(f->id);
			}
			(*fd_vec)[fd] = fd_id;
		}
		
		hash_t p_id = *(p->id);
		// delete previous process fd map
		auto it = ProcFdMap.find(p_id);
		if (it != ProcFdMap.end()) {
			delete(it->second);
		}
		ProcFdMap[p_id] = fd_vec;
		fdfile.close();
	}
	closedir(fddir);
	return true;
}

void KG::InsertEmptyFd(hash_t _p_id) {
	std::vector<hash_t> *fd_vec = new std::vector<hash_t>;
	ProcFdMap[_p_id] = fd_vec;
}

bool KG::InsertFd(hash_t _p_id, std::string _fd, hash_t _file) {
	auto it = ProcFdMap.find(_p_id);
	if (it != ProcFdMap.end()) {
		// proc was created in ProcFdMap
		std::vector<hash_t>* fd_vec;
		fd_vec = it->second;
		
		fd_t fd_vec_size = fd_vec->size();
		fd_t fd_idx = std::stoi(_fd);
		if (fd_idx < fd_vec_size) {
			(*fd_vec)[fd_idx] = _file;
			return true;
		}
		
		// Todo: fd_vector should include _f_new
		for (int i = 0; i < int(fd_idx - fd_vec_size); i++) {
			fd_vec->push_back(0);
		}
		
		fd_vec->push_back(_file);
		return false;
	} 
	else {
		std::cerr << "Insert: Cannot find parent proc " << _p_id 
		<< " when inserting ProcFdMap" << std::endl;
		return false;
	}
}

std::vector<hash_t>* KG::SearchFd(hash_t _p_id) {
	auto it = ProcFdMap.find(_p_id);
	if (it != ProcFdMap.end()) {
		return it->second;
	} 
	else {
		std::cerr << "Cannot find proc " << _p_id << " when searching ProcFdMap" << std::endl;
		return NULL;
	}
}

bool KG::CopyFd(hash_t _pp_id, hash_t _p_id) {
	std::vector<hash_t> *p_fd_vec = new std::vector<hash_t>;
	ProcFdMap[_p_id] = p_fd_vec;

	std::vector<hash_t> *pp_fd_vec = SearchFd(_pp_id);
	if (pp_fd_vec == NULL) {
		return false;
	}
	*p_fd_vec = *pp_fd_vec;
	return true;
}

bool KG::CopyFd(hash_t _p_id, std::string _f_old_str, std::string _f_new_str) {
	std::vector<hash_t> *fd_vec = SearchFd(_p_id);
	if (fd_vec == NULL) {
		return false;
	}

	fd_t _f_old = std::strtol(_f_old_str.c_str(), NULL, 16);
	fd_t _f_new = std::strtol(_f_new_str.c_str(), NULL, 10);
	
	// Todo: Auditbeat don't record syscalls for sshd 
	hash_t _file;
	fd_t fd_vec_size = fd_vec->size();
	if (_f_old < fd_vec_size) {
		_file = fd_vec->at(_f_old);
	}
	else {
		_file = 0;
	}

	if (_f_new < fd_vec_size) {
		(*fd_vec)[_f_new] = _file;
	}

	// Todo: fd_vector should include _f_new
	for (int i = 0; i < int(_f_new - fd_vec_size); i++) {
		fd_vec->push_back(0);
	}

	fd_vec->push_back(_file);
	return true;
}

bool KG::DeleteFd(hash_t _p_id, std::string _fd_str) {
	std::vector<hash_t> *fd_vec = SearchFd(_p_id);
	if (fd_vec == NULL) {
		return false;
	}

	fd_t fd_vec_size = fd_vec->size();
	fd_t _fd = std::strtol(_fd_str.c_str(), NULL, 16);
	if (_fd < fd_vec_size) {
		fd_vec->at(_fd) = 0;
	}

	// Todo: fd_vector should include _fd
	for (int i = 0; i < int(_fd - fd_vec_size); i++) {
		fd_vec->push_back(0);
	}
	return true;
}

void KG::PrintFd(hash_t _p_id) {
	std::vector<hash_t> *fd_vec = SearchFd(_p_id);
	if (fd_vec == NULL) {
		return;
	}

	std::cout << "found: " << _p_id << std::endl;
	for (size_t i = 0; i < fd_vec->size(); i++) {
		hash_t f_id = fd_vec->at(i);
		std::cout << i << ": ";
		
		NodeFile *f = SearchFile(f_id);
		
		if (f) {
			std::cout << f->name << " VER: " <<f->version;
		}
		else {
			NodeSocket *s = SearchSocket(f_id);
			if (s) {
				std::cout << s->name;
			}
		}
		std::cout << "\n";
	}
}

void KG::PrintFd() {
	for (auto it: ProcFdMap) {
		hash_t p_id = it.first;
		std::vector<hash_t> *fd_vec = it.second;

		NodeProc *p = SearchProc(p_id);
		if (p == NULL) {
			std::cerr << "Cannot find proc " << p_id << " when printing ProcFdMap" << std::endl;
			continue;
		}
		std::cout << "Proc: " << p->pid << " " << 
		p->exe << " " << p->args << " " << p_id << " has opened:" << std::endl;

		for (size_t i = 0; i < fd_vec->size(); i++) {
			hash_t f_id = fd_vec->at(i);
			std::cout << i << ": ";

			NodeFile *f = SearchFile(f_id);
			if (f) {
				std::cout << f->name << " VER: " <<f->version;
			}
			else {
				NodeSocket *s = SearchSocket(f_id);
				if (s) {
					std::cout << s->name;
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
}

void KG::InsertFileInteraction(hash_t f_hash, KGEdge *edge) {
	// 1. locate the file in the unordered_map
	auto it = FileInteractionTable.find(f_hash);
	if (it != FileInteractionTable.end()) {
		inter_map *im = it->second;
		// std::cerr << "Not first insertion for this file!" << std::endl;
		// 2. insert the edge into the inner map
		im->push_back(edge);
	}
	else {
		// create the interaction map for this file
		inter_map *im = new inter_map;
		// std::cout << "First insertion success!" << std::endl; 
		im->push_back(edge);
		FileInteractionTable[f_hash] = im;
	}
}

void KG::InsertFileInteraction(hash_t f_hash, inter_map *new_im) {
	auto f = FileInteractionTable.find(f_hash);
	if (f != FileInteractionTable.end()) {
		// insert the new im into the old im
		inter_map *old_im = f->second;
		for (auto edge: *new_im) {
			old_im->push_back(edge);
		}
		delete (new_im);
	}
	else {	
		FileInteractionTable[f_hash] = new_im;
	}
}

void KG::PrintFileInteraction() {
	for (const auto &file: FileInteractionTable) {
		std::cout << file.first << " :";
		inter_map *im = file.second;
		for (const auto &edge : *im) {
			std::string n1 = SearchNode(edge->n1_hash).second;
			std::string n2 = SearchNode(edge->n2_hash).second;

			std::cout << "sess " << edge->sess << "\tseq " << uint128tostring(edge->seq);

			std::cout << "\n" << n1;
			std::cout << "\n" << EdgeEnum2String(edge->relation);
			std::cout << "\n" << n2 << "\n" << std::endl;
		}
	}
}

void KG::InsertProcInteraction(hash_t p_hash, KGEdge *edge) {
	// 1. locate the file in the unordered_map
	auto it = ProcInteractionTable.find(p_hash);
	if (it != ProcInteractionTable.end()) {
		inter_map *im = it->second;
		// std::cerr << "Not first insertion for this file!" << std::endl;
		// 2. insert the edge into the inner map
		im->push_back(edge);
	}
	else {
		// create the interaction map for this file
		inter_map *im = new inter_map;
		// std::cout << "First insertion success!" << std::endl; 
		im->push_back(edge);
		ProcInteractionTable[p_hash] = im;
	}
}

void KG::InsertProcInteraction(hash_t p_hash, inter_map *new_im) {
	auto p = ProcInteractionTable.find(p_hash);
	if (p != ProcInteractionTable.end()) {
		// insert the new im into the old im
		inter_map *old_im = p->second;
		for (auto edge: *new_im) {
			old_im->push_back(edge);
		}
		delete (new_im);
	}
	else {	
		ProcInteractionTable[p_hash] = new_im;
	}
}

void KG::PrintProcInteraction() {
	for (const auto &proc: ProcInteractionTable) {
		std::cout << proc.first << " :";
		inter_map *im = proc.second;
		for (const auto &edge : *im) {
			std::string n1 = SearchNode(edge->n1_hash).second;
			std::string n2 = SearchNode(edge->n2_hash).second;

			std::cout << "sess " << edge->sess << "\tseq " << uint128tostring(edge->seq);

			std::cout << "\n" << n1;
			std::cout << "\n" << EdgeEnum2String(edge->relation);
			std::cout << "\n" << n2 << "\n" << std::endl;
		}
	}
}

// compute e_id (hash) for every edge
// generate KGEdgeTable based on KGEdgeList
void KG::GenKGEdgeTable() {
	std::hash<std::string> hasher;
	for (const auto edge: KGEdgeList) {
		hash_t _n1_hash = *(edge->n1_id);
		hash_t _n2_hash = *(edge->n2_id);
		edge->n1_hash = _n1_hash;
		edge->n2_hash = _n2_hash;
		std::string n1_hash = std::to_string(_n1_hash);
		std::string n2_hash = std::to_string(_n2_hash);
		std::string relation = std::to_string(EdgeEnum2Int(edge->relation));
		std::string sequence = uint128tostring(edge->seq);
		std::string session = std::to_string(edge->sess);
		std::string temp_edge_info = n1_hash + n2_hash + relation + sequence + session;
		hash_t e_id = (hash_t)hasher(temp_edge_info);
		edge->e_id = e_id;
		InsertEdge(e_id, edge);
	}
	KGEdgeList.clear();
}

void KG::GenKGEdgeTableWhenLoad() {
	for (const auto edge: KGEdgeList) {
		hash_t e_hash = edge->e_id;
		KGEdgeTable[e_hash] = edge;
	}
	KGEdgeList.clear();
}

// Delete local sockets
void KG::DeleteLocalSocket() {
	std::vector<hash_t> delete_sockets;
	std::vector<KGEdge *> delete_edges;
	std::string socket_str = "socket";

	// search for local sockets in SocketNodeTable
	for (const auto &it: SocketNodeTable) {
		NodeSocket *s = it.second;
		std::string name = s->name;
		if (name.find(socket_str) != std::string::npos) {
			hash_t s_id = *(s->id);
			delete_sockets.push_back(s_id);
		}
	}

	// search for edges accessing local sockets in SocketNodeTable
	for (const auto &it: KGEdgeTable) {
		KGEdge *edge = it.second;
		hash_t n1_hash = edge->n1_hash;
		auto n1_found = std::find(delete_sockets.begin(), delete_sockets.end(), n1_hash);
		if (n1_found != delete_sockets.end()) {
			delete_edges.push_back(edge);
			continue;
		}

		hash_t n2_hash = edge->n2_hash;
		auto n2_found = std::find(delete_sockets.begin(), delete_sockets.end(), n2_hash);
		if (n2_found != delete_sockets.end()) {
			delete_edges.push_back(edge);
		}
	}

	// delete local socket nodes
	for (auto s_id: delete_sockets) {
		KGNodeTable.erase(s_id);
		auto nh_socket = SocketNodeTable.extract(s_id);
		NodeSocket* s = nh_socket.mapped();
		delete(s);
	}

	// delete edges accessing pipes
	for (auto edge: delete_edges) {
		hash_t e_id = edge->e_id;
		auto nh_edge = KGEdgeTable.extract(e_id);
		KGEdge *e = nh_edge.mapped();
		delete(e);
	}
}

// Delete pipe nodes and create pipe edges
void KG::DeletePipeEdge() {
	std::unordered_map<hash_t, std::vector<KGEdge*>> pipe_edge_map;
	std::string pipe_str = "pipe";
	std::vector<hash_t> delete_pipes;
	std::vector<KGEdge *> delete_edges;

	// search for pipe nodes in FileNodeTable
	for (const auto &it: FileNodeTable) {
		NodeFile *f = it.second;
		std::string name = f->name;
		if (name.find(pipe_str) != std::string::npos) {
			hash_t f_id = *(f->id);
			pipe_edge_map[f_id] = std::vector<KGEdge*>();
			delete_pipes.push_back(f_id);
		}
	}

	// search for edges accessing pipes in KGEdgeTable
	for (const auto &it: KGEdgeTable) {
		KGEdge *edge = it.second;
		hash_t n1_hash = edge->n1_hash;
		auto n1_found = pipe_edge_map.find(n1_hash);
		if (n1_found != pipe_edge_map.end()) {
			std::vector<KGEdge*> &edges = n1_found->second;
			edges.push_back(edge);
			continue;
		}

		hash_t n2_hash = edge->n2_hash;
		auto n2_found = pipe_edge_map.find(n2_hash);
		if (n2_found != pipe_edge_map.end()) {
			std::vector<KGEdge*> &edges = n2_found->second;
			edges.push_back(edge);
		}
	}
	
	// create pipe edges
	for (auto it: pipe_edge_map) {
		hash_t pipe_id = it.first;
		hash_t *p1_id = NULL;
		hash_t *p2_id = NULL;
		hash_t p1_hash = 0;
		hash_t p2_hash = 0;
		std::vector<seq_t> p_seq; 
		sess_t p_sess;
		std::string p_timestamp;

		for (auto &edge: it.second) {
			hash_t n1_hash = edge->n1_hash;
			if (n1_hash == pipe_id) {
				p2_id = edge->n2_id;
				p2_hash = edge->n2_hash;
			}
			else {
				p1_id = edge->n1_id;
				p1_hash = n1_hash;
				seq_t seq = edge->seq;
				p_seq.push_back(seq);
			}
			p_sess = edge->sess;
			p_timestamp = edge->timestamp;
			delete_edges.push_back(edge);
		}

		if (p1_id != NULL and p2_id != NULL) {
			// create pipe edges
			for (auto seq: p_seq) {
				KGEdge *edge = new KGEdge (p1_id, p2_id, EdgeType_t::Pipe, seq, p_sess, p_timestamp);
				edge->n1_hash = p1_hash;
				edge->n2_hash = p2_hash;	
				std::string n1_hash = std::to_string(p1_hash);
				std::string n2_hash = std::to_string(p2_hash);
				std::string relation = std::to_string(EdgeEnum2Int(EdgeType_t::Pipe));
				std::string sequence = uint128tostring(seq);
				std::string session = std::to_string(p_sess);
				std::string temp_edge_info = n1_hash + n2_hash + relation + sequence + session;
				std::hash<std::string> hasher;
				hash_t e_id = (hash_t)hasher(temp_edge_info);
				edge->e_id = e_id;
				KGEdgeTable[e_id] = edge;
			}
		}
	}

	// delete pipe nodes
	for (auto f_id: delete_pipes) {
		KGNodeTable.erase(f_id);
		auto nh_file = FileNodeTable.extract(f_id);
		NodeFile* f = nh_file.mapped();
		delete(f);
	}

	// delete edges accessing pipes
	for (auto edge: delete_edges) {
		hash_t e_id = edge->e_id;
		auto nh_edge = KGEdgeTable.extract(e_id);
		KGEdge *e = nh_edge.mapped();
		delete(e);
	}
}

// revocer proc exe and args for process without being executed
void KG::ProcInfoRecover() {
	std::cout << "recover process info" << std::endl;
	std::unordered_map<hash_t, hash_t> p_recover;
	std::vector<NodeProc *> p_imcomplete;
	// update proc in ProcNodeMap and store proc whose parent needs to be updated in p_imcomplete
	for (auto &it: ProcNodeTable) {
		NodeProc *p = it.second;
		std::string exe = p->exe;
		// proc with exe does not need to be updated
		if (exe.compare(" ") != 0) {
			continue;
		}

		// Update the exe and args based on parent proc
		std::string ppid = p->ppid;
		NodeProc *pp = SearchProc(ppid);
		if (pp == NULL) {
			continue;
		}
		std::string pp_exe = pp->exe;
		if (pp_exe.compare(" ") == 0) {
			p_imcomplete.push_back(p);
			continue;
		}
		std::string pp_args = pp->args;
		hash_t p_old_id = *(p->id);
		p->UpdateID(pp_exe, pp_args);

		// collect procs to be recovered
		hash_t p_new_id = *(p->id);
		p_recover[p_old_id] = p_new_id;
	}

	// p1 -> clone -> p2 -> clone -> p3 -> clone -> p4 (no execve for p2,p3,p4)
	while (!p_imcomplete.empty()) {
		std::vector<NodeProc *> p_temp_imcomplete;
		p_temp_imcomplete.swap(p_imcomplete);

		for (auto &p: p_temp_imcomplete) {
			// Update the exe and args from parent proc
			std::string ppid = p->ppid;
			NodeProc *pp = SearchProc(ppid);
			std::string pp_exe = pp->exe;
			if (pp_exe.compare(" ") == 0) {
				p_imcomplete.push_back(p);
				continue;
			}
			std::string pp_args = pp->args;
			hash_t p_old_id = *(p->id);
			p->UpdateID(pp_exe, pp_args);

			// collect procs to be recovered
			hash_t p_new_id = *(p->id);
			p_recover[p_old_id] = p_new_id;
		}
	}

	// update ProcNodeMap, KGNodeMap and ProcFdMap
	for (auto &it: p_recover) {
		hash_t p_old_id = it.first;
		hash_t p_new_id = it.second;
		ExchangeMapKey(ProcNodeTable, p_old_id, p_new_id);
		ExchangeMapKey(KGNodeTable, p_old_id, p_new_id);
		ExchangeMapKey(ProcFdMap, p_old_id, p_new_id);
		ExchangeMapKey(ProcInteractionTable, p_old_id, p_new_id);
	}
}
