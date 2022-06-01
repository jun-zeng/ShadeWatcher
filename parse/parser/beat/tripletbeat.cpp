#include "tripletbeat.h"

Triplet::Triplet(const Json::Value &_event, KG* _infotbl): event(_event) {
	infotbl = _infotbl;
	// SHADEWATCHER supports 32 system calls
	syscallMap["execve"] = SyscallType_t::Execve;
	syscallMap["clone"] = SyscallType_t::Clone;
	syscallMap["vfork"] = SyscallType_t::Vfork;
	syscallMap["fork"] = SyscallType_t::Clone;
	syscallMap["open"] = SyscallType_t::Open;
	syscallMap["openat"] = SyscallType_t::Open;
	syscallMap["mq_open"] = SyscallType_t::Open;
	syscallMap["pipe"] = SyscallType_t::Pipe;
	syscallMap["pipe2"] = SyscallType_t::Pipe;
	syscallMap["dup"] = SyscallType_t::Dup;
	syscallMap["dup2"] = SyscallType_t::Dup;
	syscallMap["close"] = SyscallType_t::Close;
	syscallMap["connect"] = SyscallType_t::Connect;
	syscallMap["unlink"] = SyscallType_t::Delete;
	syscallMap["unlinkat"] = SyscallType_t::Delete;
	syscallMap["socket"] = SyscallType_t::Socket;
	syscallMap["read"] = SyscallType_t::Read;
	syscallMap["write"] = SyscallType_t::Write;
	syscallMap["recvfrom"] = SyscallType_t::Recvfrom;
	syscallMap["sendto"] = SyscallType_t::Sendto;
	syscallMap["recvmsg"] = SyscallType_t::Recvmsg;
	syscallMap["sendmsg"] = SyscallType_t::Sendmsg;
	syscallMap["recvmmsg"] = SyscallType_t::Recvmsg;
	syscallMap["sendmmsg"] = SyscallType_t::Sendmsg;
	syscallMap["mkdir"] = SyscallType_t::Mkdir;
	syscallMap["rmdir"] = SyscallType_t::Rmdir;
	syscallMap["getpeername"] = SyscallType_t::Getpeername;
	syscallMap["fcntl"] = SyscallType_t::Fcntl;
	syscallMap["rename"] = SyscallType_t::Rename;
	syscallMap["kill"] = SyscallType_t::Kill;
	syscallMap["link"] = SyscallType_t::Link;
	syscallMap["linkat"] = SyscallType_t::Link;
}

Triplet::~Triplet() {
	syscallMap.clear();
}

void Triplet::Event2triplet() {
	std::string syscall_str = Jval2str(event["auditd"]["data"]["syscall"]);

	switch(syscallMap[syscall_str]) {
		case SyscallType_t::Execve: {
			SyscallExecve();
			break;
		}
		case SyscallType_t::Clone: {
			SyscallClone();
			break;
		}
		case SyscallType_t::Vfork: {
			SyscallVfork();
			break;
		}
		case SyscallType_t::Open: {
			SyscallOpen();
			break;
		}
		case SyscallType_t::Socket: {
			SyscallSocket();
			break;
		}
		case SyscallType_t::Connect: {
			SyscallConnect();
			break;
		}
		case SyscallType_t::Pipe: {
			SyscallPipe();
			break;
		}
		case SyscallType_t::Dup: {
			SyscallDup();
			break;
		}
		case SyscallType_t::Close: {
			// Todo: Close() does not work for mmap(), the fd is still available
			// even if the program calls close() after mmap()
			// SyscallClose();
			break;
		}
		case SyscallType_t::Delete: {
			SyscallDelete();
			break;
		}
		case SyscallType_t::Read: {
			SyscallRead();
			break;
		}
		case SyscallType_t::Write: {
			SyscallWrite();
			break;
		}
		case SyscallType_t::Recvfrom: {
			SyscallRecvfrom();
			break;
		}
		case SyscallType_t::Sendto: {
			SyscallSendto();
			break;
		}
		case SyscallType_t::Mkdir: {
			SyscallMkdir();
			break;
		}
		case SyscallType_t::Rmdir: {
			SyscallRmdir();
			break;
		}
		case SyscallType_t::Sendmsg: {
			SyscallSendmsg();
			break;
		}
		case SyscallType_t::Recvmsg: {
			SyscallRecvmsg();
			break;
		}
		case SyscallType_t::Getpeername: {
			SyscallGetpeername();
			break;
		}
		case SyscallType_t::Fcntl: {
			SyscallFcntl();
			break;
		}
		case SyscallType_t::Rename: {
			SyscallRename();
			break;
		}
		case SyscallType_t::Kill: {
			SyscallKill();
			break;
		}
		case SyscallType_t::Link: {
			SyscallLink();
			break;
		}
		default:
			break;
	}
}

void Triplet::SyscallVfork() {
	std::string pid = Jval2str(event["auditd"]["data"]["exit"]); 
	std::string ppid = Jval2str(event["process"]["pid"]); 
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	// search for parent process
	NodeProc *pp = infotbl->SearchProc(ppid);
	if (!pp){
		db_print("Sequence: " << uint128tostring(seq) << " vfork cannot find parent proc " << ppid);
		return;
	}
	hash_t *pp_id = pp->id;

	NodeProc *p = infotbl->SearchProc(pid);
	hash_t *p_id;
	if (p != NULL) {
		// vforked process exists before vfork syscall -> no need to create new process
		p_id = p->id;
	}
	else {
		// add new proc into infotable: ProcNodeTable, PidTable, and ProcFdMap
		NodeProc *p_temp = new NodeProc(pid);
		p_temp->ppid = ppid;
		p = infotbl->InsertProc(p_temp);
		p_id = p->id;
		// pidtable in Insertpid is deisgned for internal proc lookup
		infotbl->InsertPid(pid, p);
		// create fd list to ProcFdMap
		infotbl->CopyFd(*pp_id, *p_id);
	}

	// add new edge into KGEdge
	KGEdge *e = new KGEdge (pp_id, p_id, EdgeType_t::Vfork, seq, sess, timestamp);
	infotbl->InsertEdge(e);
}

void Triplet::SyscallClone() {
	std::string pid = Jval2str(event["auditd"]["data"]["exit"]); 
	std::string ppid = Jval2str(event["process"]["pid"]); 
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	NodeProc *p_temp = new NodeProc(pid);
	p_temp->ppid = ppid;
	
	// add new proc into infotable: ProcNodeTable, PidTable, and ProcFdMap
	NodeProc *p = infotbl->InsertProc(p_temp);
	infotbl->InsertPid(pid, p);

	hash_t *p_id = p->id;
	NodeProc *pp = infotbl->SearchProc(ppid);
	if (pp) {
		hash_t *pp_id = pp->id;
		// create fd list to ProfFdMap
		infotbl->CopyFd(*pp_id, *p_id);
		// add new edge into KGEdge
		KGEdge *e = new KGEdge (pp_id, p_id, EdgeType_t::Clone, seq, sess, timestamp);
		infotbl->InsertEdge(e);
	}
	else {
		infotbl->InsertEmptyFd(*p_id);
		db_print("Sequence: " << uint128tostring(seq) << " clone cannot find parent proc " << ppid);
	}
}

void Triplet::SyscallExecve() {
	std::string pid = Jval2str(event["process"]["pid"]); 
	std::string ppid = Jval2str(event["process"]["ppid"]);
	std::string exe = Jval2str(event["process"]["exe"]); 
	std::string args;
	if (event["process"].isMember("args")) {
		args = Jval2strArgs(event["process"]["args"]); 
		args.erase(std::remove(args.begin(), args.end(), '\"'), args.end());
		args.erase(std::remove(args.begin(), args.end(), '\\'), args.end());
		std::replace(args.begin(), args.end(), ',', ' ');
	}
	else {
		args = "null";
	}
	seq_t seq = Jval2int(event["auditd"]["sequence"]);

	// pp_id and p_id are used to create KG edge
	hash_t *pp_id = NULL;
	hash_t *p_id = NULL;

	NodeProc *p = infotbl->SearchProc(pid);

	if (p == NULL) {
		// the process does not exist:
		// add new proc into infotable: ProcNodeTable, PidTable, and ProcFdMap
		NodeProc *pp = infotbl->SearchProc(ppid);
		if (!pp){
			db_print("Sequence: " << uint128tostring(seq) << " execve cannot find parent proc " << ppid);
			return;
		}

		NodeProc *p_temp = new NodeProc(pid, exe, args, ppid);
		NodeProc *p_new = infotbl->InsertProc(p_temp);
		infotbl->InsertPid(pid, p_new);
		p_id = p_new->id;
		pp_id = pp->id;
		infotbl->CopyFd(*pp_id, *p_id);
	}
	else if (p->args == " ") {
		// the process has been created (e.g., cloned) but never executed
		hash_t p_id_old = *(p->id);
		p->UpdateID(exe, args);

		// update ProcNodeTable, KGNodeTable, ProcFdMap, and PidTable
		hash_t p_new_id = *(p->id);
		ExchangeMapKey(infotbl->KGNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(infotbl->ProcNodeTable, p_id_old, p_new_id);
		ExchangeMapKey(infotbl->ProcFdMap, p_id_old, p_new_id);
		ExchangeMapKey(infotbl->ProcInteractionTable, p_id_old, p_new_id);

		NodeProc *pp = infotbl->SearchProc(ppid);
		if (!pp){
			db_print("Sequence: " << uint128tostring(seq) << " execve cannot find parent proc " << ppid);
			return;
		}
		p_id = p->id;
		pp_id = pp->id;
	}
	else {
		// the process has been executed
		NodeProc *p_temp = new NodeProc(pid, exe, args, ppid);
		NodeProc *p_new = infotbl->InsertProc(p_temp);
		p_id = p_new->id;
		pp_id = p->id;

		// update PidTable and insert ProcFdMap
		auto it = infotbl->PidTable.find(pid);
		it->second = p_new;
		infotbl->CopyFd(*pp_id, *p_id);
	}

	// add new file into NodeFileTable
	std::vector <hash_t *> fileID;

	std::string name;
	std::string version;
	for (auto file : event["auditd"]["paths"]){
		name = Jval2str(file["name"]);
		version = Jval2str(file["version"]);

		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}

		NodeFile *f_tmp = new NodeFile (name, version);
		NodeFile *f = infotbl->InsertFile(f_tmp);
		hash_t *f_id = f->id;
		fileID.push_back(f_id);
	}

	// Todo: add new attr into NodeAttrTable

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	for (hash_t *f_id_ptr : fileID){
		KGEdge *e = new KGEdge (f_id_ptr, p_id, EdgeType_t::Load, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*f_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id, e);
	}

	KGEdge *e = new KGEdge (pp_id, p_id, EdgeType_t::Execve, seq, sess, timestamp);
	infotbl->InsertEdge(e);
}

void Triplet::SyscallOpen() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::string fd = Jval2str(event["auditd"]["data"]["exit"]);

	// create new file into FileNodeTable
	std::string name;
	std::string version;

	int create_flag = 0;
	std::string nametype;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);
		if (nametype.compare("PARENT") == 0) {
			continue;
		}
		if (! file.isMember("name")) {
			continue;
		}
		name = Jval2str(file["name"]);
		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		version = Jval2str(file["version"]);
		if (nametype.compare("CREATE") == 0){
			create_flag = 1;
		}
	}
	
	NodeFile *f_tmp = new NodeFile (name, version);
	NodeFile *f = infotbl->InsertFile(f_tmp);
	hash_t fd_id = *(f->id);
	infotbl->InsertFd(p_id, fd, fd_id);

	// add create edge into KGEdge	
	if (create_flag) {
		seq_t seq = Jval2int(event["auditd"]["sequence"]);
		sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
		std::string timestamp = Jval2str(event["@timestamp"]);

		hash_t *p_id_ptr = p->id;
		hash_t *f_id_ptr = f->id;
		
		KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Create, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*f_id_ptr, e);
	}
}

void Triplet::SyscallSocket() {	
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);
	
	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::string fd = Jval2str(event["auditd"]["data"]["exit"]);
	
	// create new socket into SocketNodeTable
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string name = "socket" + uint128tostring(seq);

	NodeSocket *s_tmp = new NodeSocket (name);
	NodeSocket *s = infotbl->InsertSocket(s_tmp);
	hash_t fd_id = *(s->id);
	infotbl->InsertFd(p_id, fd, fd_id);

	// add new edges into KGEdge
	// sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));

	// hash_t *p_id_ptr = p->id;
	// hash_t *f_id_ptr = s->id;

	// KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Socket, seq, sess);
	// infotbl->InsertEdge(e);
}

void Triplet::SyscallConnect() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	
	// obtain old file id
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);		
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);

	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	// hash_t f_id = (*fd_vec)[fd_idx];

	// to update the socket name
	std::string ip = Jval2str(event["destination"]["ip"]);
	std::string port = Jval2str(event["destination"]["port"]);
	std::string new_name = ip + ":" + port;

	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		(*fd_vec)[fd_idx] = -1;
		return;
	}

	NodeSocket *s_tmp = new NodeSocket (new_name);
	NodeSocket *s = infotbl->InsertSocket(s_tmp);

	(*fd_vec)[fd_idx] = *(s->id);

	// Todo: We assume that there is no edge (e.g., sendto, recvefrom) including old socket before connect
	// infotbl->SocketNodeTable.extract(f_id);
	// infotbl->KGNodeTable.extract(f_id);

	// add new edges into KGEdge (debug)
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);

	hash_t *p_id_ptr = p->id;
	hash_t *s_id_ptr = s->id;
	KGEdge *e = new KGEdge (p_id_ptr, s_id_ptr, EdgeType_t::Connect, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*s_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallPipe() {
	std::string pid = Jval2str(event["process"]["pid"]);
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	// pipe does not specify fd
	if (!event["auditd"]["data"]["fd0"]) {
		return;
	}

	hash_t p_id = *(p->id);
	std::string fd_1 = Jval2str(event["auditd"]["data"]["fd0"]);
	std::string fd_2 = Jval2str(event["auditd"]["data"]["fd1"]);

	// create new pipe into FileNodeTable
	std::string name = "pipe" + uint128tostring(seq);
	NodeFile *f_tmp = new NodeFile (name);
	NodeFile *f = infotbl->InsertFile(f_tmp);
	hash_t f_id = *(f->id);

	infotbl->InsertFd(p_id, fd_1, f_id);
	infotbl->InsertFd(p_id, fd_2, f_id);

	// do not need to add pipe edges
	// KGEdge *e_in = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Pipe, seq, sess);
}

void Triplet::SyscallDup() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);	
	std::string fd_old = Jval2str(event["auditd"]["data"]["a0"]);
	std::string fd_new = Jval2str(event["auditd"]["data"]["exit"]);

	// copy new fd into PidTable
	infotbl->CopyFd(p_id, fd_old, fd_new);
}


void Triplet::SyscallClose() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);

	std::string fd = Jval2str(event["auditd"]["data"]["a0"]);
	infotbl->DeleteFd(p_id, fd);
}

void Triplet::SyscallMkdir() {
	std::string name;
	std::string version;

	// If it is a relative path (1) or absolute path (2)
	std::string nametype;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);

		if (nametype.compare("PARENT") == 0) {
			continue;
		}

		name = Jval2str(file["name"]);
		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		version = Jval2str(file["version"]);
	}

	NodeFile *f_tmp = new NodeFile (name, version);
	NodeFile *f = infotbl->InsertFile(f_tmp);
	
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	hash_t *p_id_ptr = p->id;
	hash_t *f_id_ptr = f->id;

	KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Mkdir, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*f_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallRmdir() {
	std::string name;
	std::string version;
	std::string nametype;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);

		if (nametype.compare("PARENT") == 0) {
			continue;
		}

		name = Jval2str(file["name"]);
		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		version = Jval2str(file["version"]);
	}

	NodeFile *f_tmp = new NodeFile(name, version);
	NodeFile *f = infotbl->InsertFile(f_tmp);

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);
	
	// magic proc is never created 
	if (p == NULL) {
		return;
	}
	
	hash_t *f_id_ptr = f->id;
	hash_t *p_id_ptr = p->id;

	KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Rmdir, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*f_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallDelete() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	std::string name;
	std::string version;
	std::string nametype;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);

		if (nametype.compare("PARENT") == 0) {
			continue;
		}

		name = Jval2str(file["name"]);
		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		version = Jval2str(file["version"]);
	}

	NodeFile *f_tmp = new NodeFile(name, version);
	NodeFile *f = infotbl->InsertFile(f_tmp);

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);
	
	hash_t *p_id_ptr = p->id;
	hash_t *f_id_ptr = f->id;

	KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Delete, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*f_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallRead() {
	// Don't track read system with 0 bytes return
	std::string read_bytes = Jval2str(event["auditd"]["data"]["exit"]);	
	if (read_bytes.compare("0") == 0) {
		return;
	}
	
	// event sequence
	seq_t seq = Jval2int(event["auditd"]["sequence"]);

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);	
	fd_t fd_vec_size = fd_vec->size();
	// Todo: we don't record syscalls for sshd as events with auid=-1 is
	// filtered. As a result, we might miss pipe syscalls for IPC between sshd and new
	// ssh session	
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " unexpected fd: " << fd_idx << " in read syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];
	// Todo: we don't record syscalls for sshd as events with auid=-1 is
	// filtered. As a result, we might miss pipe syscalls for IPC between sshd and new
	// ssh session	
	if (f_id == 0 && fd_idx != 0) {
		db_print("Sequence: " << uint128tostring(seq) << " read closed fd: " << fd_idx << " in read syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	hash_t *p_id_ptr = p->id;
	NodeFile *f = infotbl->SearchFile(f_id);

	if (f == NULL) {
		NodeSocket *s = infotbl->SearchSocket(f_id);
		if (s == NULL) {
			return;
		}
		hash_t *s_id_ptr = s->id;
		KGEdge *e = new KGEdge (s_id_ptr, p_id_ptr, EdgeType_t::Recv, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*s_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id_ptr, e);
	}
	else {
		hash_t *f_id_ptr = f->id;
		KGEdge *e = new KGEdge (f_id_ptr, p_id_ptr, EdgeType_t::Read, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*f_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id_ptr, e);
	}
}

void Triplet::SyscallWrite() {
	// Don't track write system with 0 bytes return
	std::string write_bytes = Jval2str(event["auditd"]["data"]["exit"]);	
	if (write_bytes.compare("0") == 0) {
		return;
	}

	// event sequence
	seq_t seq = Jval2int(event["auditd"]["sequence"]);

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	fd_t fd_vec_size = fd_vec->size();
	// Todo: Auditbeat don't record syscalls for sshd 
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " unexpected fd: " << fd_idx << " in write syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];
	// Todo: Auditbeat don't record syscalls for sshd 
	if (f_id == 0 && fd_idx != 1 && fd_idx != 2) {
		db_print("Sequence: " << uint128tostring(seq) << " write closed fd: " << fd_idx << " in write syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	hash_t *p_id_ptr = p->id;
	NodeFile *f = infotbl->SearchFile(f_id);
	if (f == NULL) {
		NodeSocket *s = infotbl->SearchSocket(f_id);
		if (s == NULL) {
			return;
		}
		hash_t *s_id_ptr = s->id;
		KGEdge *e = new KGEdge (p_id_ptr, s_id_ptr, EdgeType_t::Send, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*s_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id_ptr, e);
	}
	else {
		hash_t *f_id_ptr = f->id;
		KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Write, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*f_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id_ptr, e);
	}
}

void Triplet::SyscallRecvfrom() {
	// Don't track recvefrom system with 0 bytes return
	std::string recvfrom_bytes = Jval2str(event["auditd"]["data"]["exit"]);	
	if (recvfrom_bytes.compare("0") == 0) {
		return;
	}

	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	fd_t fd_vec_size = fd_vec->size();

	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	// Todo: Auditbeat don't record syscalls for sshd 
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " Unexpected fd: " << fd_idx << " in recvfrom syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];
	// Todo: Auditbeat don't record syscalls for sshd 
	if (f_id == 0 && fd_idx != 0) {
		db_print("Sequence: " << uint128tostring(seq) << " read closed fd: " << fd_idx << " in recvfrom syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	NodeSocket *s = infotbl->SearchSocket(f_id);
	if (s == NULL) {
		return;
	}
	
	hash_t *s_id_ptr = s->id;
	hash_t *p_id_ptr = p->id;

	KGEdge *e = new KGEdge (s_id_ptr, p_id_ptr, EdgeType_t::Recv, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*s_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallSendto() {
	// Don't track sendto system with 0 bytes return
	std::string sendto_bytes = Jval2str(event["auditd"]["data"]["exit"]);	
	if (sendto_bytes.compare("0") == 0) {
		return;
	}

	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	fd_t fd_vec_size = fd_vec->size();

	// Todo: Auditbeat don't record syscalls for sshd 
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " Unexpected fd: " << fd_idx << " in sendto syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];
	// Todo: Auditbeat don't record syscalls for sshd 
	if (f_id == 0 && fd_idx != 1) {
		db_print("Sequence: " << uint128tostring(seq) << " send closed fd: " << fd_idx << " in sendto syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	NodeSocket *s = infotbl->SearchSocket(f_id);
	if (s == NULL) {
		return;
	}

	hash_t *s_id_ptr = s->id;
	hash_t *p_id_ptr = p->id;

	KGEdge *e = new KGEdge (p_id_ptr, s_id_ptr, EdgeType_t::Send, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*s_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallSendmsg() {
	// Don't track sendmsg system with 0 bytes return
	std::string sendmsg_bytes = Jval2str(event["auditd"]["data"]["exit"]);	
	if (sendmsg_bytes.compare("0") == 0) {
		return;
	}

	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	fd_t fd_vec_size = fd_vec->size();

	// Todo: Auditbeat don't record syscalls for sshd 
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " Unexpected fd: " << fd_idx << " in sendmsg syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];

	// Todo: Auditbeat don't record syscalls for sshd 
	if (f_id == 0 && fd_idx != 1) {
		db_print("Sequence: " << uint128tostring(seq) << " send closed fd: " << fd_idx << " in sendmsg syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	NodeSocket *s = infotbl->SearchSocket(f_id);
	if (s == NULL) {
		return;
	}

	// Sendmsg could happen before Connect Syscall
	if (s->name.find("socket") != std::string::npos) {
		if (event["auditd"]["data"]["socket"].isMember("addr")) {
			std::string new_ip = Jval2str(event["auditd"]["data"]["socket"]["addr"]);
			std::string new_port = Jval2str(event["auditd"]["data"]["socket"]["port"]);
			s->name = new_ip + ":" + new_port;
		}
	}

	hash_t *s_id_ptr = s->id;
	hash_t *p_id_ptr = p->id;

	KGEdge *e = new KGEdge (p_id_ptr, s_id_ptr, EdgeType_t::Send, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*s_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallRecvmsg() {
	// Don't track recvmsg system with 0 bytes return
	std::string recvmsg = Jval2str(event["auditd"]["data"]["exit"]);	
	if (recvmsg.compare("0") == 0) {
		return;
	}

	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);	
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	fd_t fd_vec_size = fd_vec->size();

	// Todo: Auditbeat don't record syscalls for sshd 
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	if (fd_idx >= fd_vec_size) {
		db_print("Sequence: " << uint128tostring(seq) << " Unexpected fd: " << fd_idx << " in recvmsg syscall"); 
		return;
	}

	hash_t f_id = (*fd_vec)[fd_idx];
	// Todo: Auditbeat don't record syscalls for sshd 
	if (f_id == 0 && fd_idx != 0) {
		db_print("Sequence: " << uint128tostring(seq) << " recv closed fd: " << fd_idx << " in recvmsg syscall"); 
		return;
	}

	// add new edges into KGEdge
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);

	// Recvmsg could happen before Connect Syscall
	NodeSocket *s = infotbl->SearchSocket(f_id);
	if (s == NULL) {
		return;
	}
	
	if (s->name.find("socket") != std::string::npos) {
		if (event["auditd"]["data"]["socket"].isMember("addr")) {
			std::string new_ip = Jval2str(event["auditd"]["data"]["socket"]["addr"]);
			std::string new_port = Jval2str(event["auditd"]["data"]["socket"]["port"]);
			s->name = new_ip + ":" + new_port;
		}
	}

	hash_t *s_id_ptr = s->id;
	hash_t *p_id_ptr = p->id;

	KGEdge *e = new KGEdge (s_id_ptr, p_id_ptr, EdgeType_t::Recv, seq, sess, timestamp);
	infotbl->InsertEdge(e);
	infotbl->InsertFileInteraction(*s_id_ptr, e);
	infotbl->InsertProcInteraction(*p_id_ptr, e);
}

void Triplet::SyscallGetpeername() {
	// We dont track internel socket
	Json::Value socket = event["auditd"]["data"]["socket"];
	if (socket.isMember("saddr") || socket.isMember("path")) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);
	
	// obtain old file id
	std::string fd_str = Jval2str(event["auditd"]["data"]["a0"]);		
	fd_t fd_idx = std::strtol(fd_str.c_str(), NULL, 16);
	std::vector<hash_t>* fd_vec = infotbl->SearchFd(p_id);
	// hash_t f_id = (*fd_vec)[fd_idx];

	// to update the socket name
	std::string ip = Jval2str(event["auditd"]["data"]["socket"]["addr"]);
	std::string port = Jval2str(event["auditd"]["data"]["socket"]["port"]);
	std::string new_name = ip + ":" + port;

	NodeSocket *s_tmp = new NodeSocket (new_name);
	NodeSocket *s = infotbl->InsertSocket(s_tmp);

	(*fd_vec)[fd_idx] = *(s->id);

	// Todo: We assume that there is no edge (e.g., sendto, recvefrom) including old socket before getpeername
	// infotbl->SocketNodeTable.extract(f_id);
	// infotbl->KGNodeTable.extract(f_id);

	// add new edges into KGEdge (debug)
	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);

	hash_t *p_id_ptr = p->id;
	hash_t *s_id_ptr = s->id;
	KGEdge *e = new KGEdge (p_id_ptr, s_id_ptr, EdgeType_t::Getpeername, seq, sess, timestamp);
	infotbl->InsertEdge(e);
}

void Triplet::SyscallFcntl() {
	// F_DUPFD = 0
	std::string args = Jval2str(event["auditd"]["data"]["a1"]);
	if (args.compare("0") != 0) {
		return;
	}

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t p_id = *(p->id);	
	std::string fd_old = Jval2str(event["auditd"]["data"]["a0"]);
	std::string fd_new = Jval2str(event["auditd"]["data"]["exit"]);

	// copy new fd into PidTable
	infotbl->CopyFd(p_id, fd_old, fd_new);
}

void Triplet::SyscallRename() {
	std::string name;
	std::string version;
	std::string nametype;

	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	if (p == NULL) {
		return;
	}

	hash_t *p_id_ptr = p->id;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);

		if (nametype.compare("PARENT") == 0) {
			continue;
		}

		name = Jval2str(file["name"]);
		version = Jval2str(file["version"]);

		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		
		NodeFile *f_tmp = new NodeFile (name, version);
		NodeFile *f = infotbl->InsertFile(f_tmp);

		hash_t *f_id_ptr = f->id;

		if (nametype.compare("CREATE") == 0) {
			KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Create, seq, sess, timestamp);
			infotbl->InsertEdge(e);
			infotbl->InsertFileInteraction(*f_id_ptr, e);
			infotbl->InsertProcInteraction(*p_id_ptr, e);
		}
		else {
			KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Delete, seq, sess, timestamp);
			infotbl->InsertEdge(e);
			infotbl->InsertFileInteraction(*f_id_ptr, e);
			infotbl->InsertProcInteraction(*p_id_ptr, e);
		}
	}
}

void Triplet::SyscallKill() {
	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);
	seq_t seq = Jval2int(event["auditd"]["sequence"]);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	std::string delete_pid_hex = Jval2str(event["auditd"]["data"]["a0"]);
	proc_t delete_pid_int = std::strtol(delete_pid_hex.c_str(), NULL, 16);

	// Todo: We do not consider the situation that pid in kill is not positive
	// 2147483647 = 2^31 - 1
	if (delete_pid_int > 2147483647) {
		return;
	}
	std::string delete_pid_str = std::to_string(delete_pid_int);

	NodeProc *delete_p = infotbl->SearchProc(delete_pid_str);
	if (delete_p == NULL) {
		db_print("Sequence: " << uint128tostring(seq) << "cannot find killed proc: " << delete_pid_str);
		return;
	}

	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	std::string timestamp = Jval2str(event["@timestamp"]);
	hash_t *p_id_ptr = p->id;
	hash_t *delete_p_id_ptr = delete_p->id;

	KGEdge *e = new KGEdge (p_id_ptr, delete_p_id_ptr, EdgeType_t::Kill, seq, sess, timestamp);
	infotbl->InsertEdge(e);
}

void Triplet::SyscallLink() {
	std::string name;
	std::string version;
	std::string nametype;

	sess_t sess = std::stoi(Jval2str(event["auditd"]["session"]));
	seq_t seq = Jval2int(event["auditd"]["sequence"]);
	std::string timestamp = Jval2str(event["@timestamp"]);

	std::string pid = Jval2str(event["process"]["pid"]);
	NodeProc *p = infotbl->SearchProc(pid);

	// magic proc is never created 
	if (p == NULL) {
		return;
	}

	hash_t *p_id_ptr = p->id;

	for (auto file : event["auditd"]["paths"]){
		nametype = Jval2str(file["nametype"]);

		if (nametype.compare("CREATE") != 0) {
			continue;
		}

		name = Jval2str(file["name"]);
		version = Jval2str(file["version"]);
		if (name[0] == '.') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + name.substr(1);
		}
		else if (name[0] != '/') {
			std::string dir = Jval2str(event["process"]["cwd"]);
			name = dir + "/" + name;
		}
		
		NodeFile *f_tmp = new NodeFile (name, version);
		NodeFile *f = infotbl->InsertFile(f_tmp);

		hash_t *f_id_ptr = f->id;

		KGEdge *e = new KGEdge (p_id_ptr, f_id_ptr, EdgeType_t::Create, seq, sess, timestamp);
		infotbl->InsertEdge(e);
		infotbl->InsertFileInteraction(*f_id_ptr, e);
		infotbl->InsertProcInteraction(*p_id_ptr, e);
	}
}
