#ifndef SHADEWATCHER_PARSER_DARPA_TRIPLETDAR_H_
#define SHADEWATCHER_PARSER_DARPA_TRIPLETDAR_H_

#include "parser/kg.h"
#include <set>

class Triplet
{
public:
	const Json::Value &event;
	KG *infotbl;
	// 0: trace; 1: theia (different Ubuntu version)
	int dataset_type;
	// Event to be analyzed; We do not parse every event
	event_t event_analyzed;
	std::unordered_map <std::string, SyscallType_t> syscallMap;

	Triplet(const Json::Value &, KG *, int);
	~Triplet();
	
	void LoadProc();
	void LoadFile();
	void LoadSock();

	void Event2triplet();
	void SyscallClone(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallExecve(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallOpen(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallRead(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallWrite(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallRecvfrom(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallSendto(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallConnect(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallDelete(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallRename(seq_t, sess_t, hash_t, hash_t, std::string, std::string, std::string);
	void SyscallSendmsg(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallRecvmsg(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallSend(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallRecv(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallLoad(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallCreate(seq_t, sess_t, hash_t, hash_t, hash_t, std::string);
	void SyscallUpdate(seq_t, sess_t, hash_t, hash_t, std::string, std::string, std::string);
};

#endif
