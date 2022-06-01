#ifndef SHADEWATCHER_PARSER_BEAT_TRIPLETBEAT_H_
#define SHADEWATCHER_PARSER_BEAT_TRIPLETBEAT_H_

#include "parser/kg.h"

class Triplet
{
public:
	const Json::Value &event;
	KG *infotbl;

	std::unordered_map <std::string, SyscallType_t> syscallMap;

	Triplet(const Json::Value &, KG *);
	~Triplet();
	
	void Event2triplet();

	void SyscallClone();
	void SyscallVfork();
	void SyscallExecve();
	void SyscallKill();
	void SyscallOpen();
	void SyscallPipe();
	void SyscallDup();
	void SyscallClose();
	void SyscallRead();
	void SyscallWrite();
	void SyscallRecvfrom();
	void SyscallSendto();
	void SyscallConnect();
	void SyscallSocket();
	void SyscallDelete();
	void SyscallLink();
	void SyscallRename();
	void SyscallMkdir();	
	void SyscallRmdir();
	void SyscallSendmsg();
	void SyscallRecvmsg();
	void SyscallGetpeername();
	void SyscallFcntl();

};

#endif
