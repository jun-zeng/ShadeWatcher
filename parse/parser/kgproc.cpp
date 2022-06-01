#include "kgproc.h"

NodeProc::NodeProc(std::string _pid, std::string _exe, std::string _args, 
	std::string _ppid, hash_t _hash): NodeKG(NodeType_t::Proc) {
	pid = _pid;
	exe = _exe;
	args = _args;
	ppid = _ppid;
	*id = _hash;
}

NodeProc::NodeProc(std::string _pid, std::string _exe, std::string _args, 
	std::string _ppid, std::string _uuid): NodeKG(NodeType_t::Proc) {
	pid = _pid;
	exe = _exe;
	args = _args;
	ppid = _ppid;

	// hash(pid + exe + args) ==> id
	std::hash<std::string> hasher;
	// std::string str = pid + exe + args;
	*id = (hash_t)hasher(_uuid);
}

NodeProc::NodeProc(std::string _exe, hash_t _id): NodeKG(NodeType_t::Proc) {
	exe = _exe;
	*id = _id;
}

NodeProc::NodeProc(std::string _pid, std::string _exe, std::string _args, 
	std::string _ppid): NodeKG(NodeType_t::Proc) {
	pid = _pid;
	exe = _exe;
	args = _args;
	ppid = _ppid;

	// hash(pid + exe + args) ==> id
	std::hash<std::string> hasher;
	std::string str = pid + exe + args;
	*id = (hash_t)hasher(str);
}

NodeProc::NodeProc(std::string _pid, std::string _exe): NodeKG(NodeType_t::Proc) {
	pid = _pid;
	exe = _exe;
	args = " ";
	ppid = " ";

	// hash(pid + exe + args) ==> id
	std::hash<std::string> hasher;
	std::string str = pid + exe + args;
	*id = (hash_t)hasher(str);
}

NodeProc::NodeProc(std::string _pid): NodeKG(NodeType_t::Proc) {
	pid = _pid;
	exe = " ";
	args = " ";
	ppid = " ";

	// hash(pid + " " + " ") ==> id
	std::hash<std::string> hasher;
	std::string str = pid + exe + args;
	*id = (hash_t)hasher(str);
}

hash_t NodeProc::ComputeID(std::string _pid){
	std::hash<std::string> hasher;
	std::string str = _pid + " " + " ";
	return (hash_t)hasher(str);
}

void NodeProc::UpdateID(std::string _exe, std::string _args) {
	exe = _exe;
	args = _args;

	std::hash<std::string> hasher;
	std::string str = pid + exe + args;
	*id = (hash_t)hasher(str);
}

void NodeProc::PrintProc(){
	std::cout << "pid " << pid;
	std::cout << "\texe " << exe;
	std::cout << "\targs " << args;
	std::cout << "\tppid " << ppid << std::endl;
}
