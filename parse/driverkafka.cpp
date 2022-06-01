#include "driverkafka.h"

int main(int argc, char **argv)
{
    // parse command line arguments
	Config cfg(argc, argv);
	cfg.ConfigKafka();
	
	// knowledge graph for auditbeat
	KG *infotbl = new KG(auditbeat);

	if (cfg.loadfromdb) {
		// load knowledge graph from local files
		KGLoadAllFromDB(cfg.postgres_config, infotbl, cfg.topic);
		
		// print KG information
		infotbl->PrintKG();
	}
	else {
		// load system info before audting starts
		Loadmetainfo(cfg.auditbeat_data_dir, infotbl);
		
		KGKafkaParse(infotbl, cfg);

		// print KG information
		infotbl->PrintKG();
	}

	// visualize in neo4j
	if (cfg.graph) {
    	Neo4jdb neo4jdb(cfg.neo4j_config, infotbl);
		neo4jdb.Neo4jVisKG();
	}

	infotbl->FreeInteraction();
	infotbl->FreeNode();
	delete infotbl;
	return 0;
}
