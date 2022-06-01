#ifndef SHADEWATCHER_PARSER_KGNODE_H_
#define SHADEWATCHER_PARSER_KGNODE_H_

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <list>
#include <algorithm>
#include <sstream>
#include "util/normalize.h"
#include "common.h"

class NodeKG {
public:
	// Hash value is node identifier
	hash_t *id;
	NodeType_t type;

	NodeKG(NodeType_t _type);
	~NodeKG();
};

#endif
