#ifndef SHADEWATCHER_PARSER_KGSOCKET_H_
#define SHADEWATCHER_PARSER_KGSOCKET_H_

#include "kgnode.h"

class NodeSocket: public NodeKG {
public:
	// identifier: name
	// internel socket: name = socket:[4941446]
	// externel socket: name = ip + ":" + port
	std::string name;

	// init socket node in socket syscall
	NodeSocket(std::string _name);
	NodeSocket(std::string _name, hash_t _id);
	NodeSocket(std::string _name, std::string _uuid);

	void UpdateID(std::string _new_name);
	void PrintSocket();
};

#endif
