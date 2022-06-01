#ifndef SHADEWATCHER_PARSER_KGPROC_H_
#define SHADEWATCHER_PARSER_KGPROC_H_

#include "kgnode.h"

class NodeProc: public NodeKG {
public:
	// identifier: pid + exe + args
	std::string pid;
	std::string exe;
	std::string args;

	std::string ppid;

	// init proc node in execve/clone syscall
	NodeProc(std::string _pid);
	NodeProc(std::string _pid, std::string _exe);
	NodeProc(std::string _exe, hash_t _id);
	NodeProc(std::string _pid, std::string _exe, std::string _args, std::string _ppid);
	NodeProc(std::string _pid, std::string _exe, std::string _args, std::string _ppid, std::string _uuid);
	NodeProc(std::string _pid, std::string _exe, std::string _args, std::string _ppid, hash_t _hash);

	// compute hash ID for Proc without exe and args
	static hash_t ComputeID(std::string _pid);
	void UpdateID(std::string _exe, std::string _args);
	void PrintProc();
};

#endif
