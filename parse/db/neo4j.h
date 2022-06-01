#ifndef SHADEWATCHER_DB_NEO4J_H_
#define SHADEWATCHER_DB_NEO4J_H_

extern "C" {
	#include <neo4j-client.h>
};

#include <errno.h>
#include "parser/kg.h"
#include <cstdio>
#include "util/config.h"

class Neo4jdb
{
public:
	neo4j_connection_t *connection;
	KG *infotbl;
	int batch_edge;
	int batch_node;

	Neo4jdb(std::string, KG *);
	~Neo4jdb();
	
	std::string Neo4jSearchNode(hash_t);
	std::string run(std::string);
	void Cleandb();
	bool Createnode(std::string);
	std::string EdgeCmd(KGEdge *, event_t);
	std::string NodeCmd(hash_t, nodenum_t);
	void Neo4jVisKG();
	void Neo4jVizEdge(std::vector<KGEdge*>);
	void Neo4jPrint();
};

#endif
