#ifndef SHADEWATCHER_PARSER_KGFILE_H_
#define SHADEWATCHER_PARSER_KGFILE_H_

#include "kgnode.h"

class NodeFile: public NodeKG {
public:
	// identifier: name = path+name
	std::string name;
	
	// Every time the inode is modified, the i_version field will be incremented.
	std::string version;

	// init file node in open/execve/.. syscall
	NodeFile(std::string _name);
	NodeFile(std::string _name, std::string _version);
	NodeFile(std::string _name, hash_t _id);
	NodeFile(std::string _name, std::string _version, std::string _hash);
	NodeFile(std::string _name, std::string _version, hash_t _hash);

   	static hash_t ComputeID(std::string, std::string);
	void UpdateID();
	void PrintFile();
};

#endif
