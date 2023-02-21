#include "driverbeat.h"

int main(int argc, char **argv)
{
	// parse command line arguments
	Config cfg(argc, argv);
	cfg.ConfigBeat();
	
	// Knowledge graph for auditbeat
	KG *infotbl = new KG(auditbeat);

	// define Local File to store KG (no computation)
	LocalStore ls(cfg.embed_data_path, infotbl);

	// visualize in neo4j
	Neo4jdb neo4jdb(cfg.neo4j_config, infotbl);

	if (cfg.loadentity) {
		// load system entities from local files
		EntityLoadFromFile(cfg.embed_data_path, infotbl);
		// print KG information
		infotbl->PrintKG();
	}

	int file_id = 0;
	auto beat_dirs = TraverseBeatDir(cfg.auditbeat_data_dir);
	for (auto beat_dir : beat_dirs) {
		std::cout << "Processing Dir: " << beat_dir << std::endl;
		
		// load system info before audting starts
		Loadmetainfo(beat_dir, infotbl);
		
		auto beat_files = CollectBeatFile(beat_dir);
		for (auto beat_file: beat_files) {
			KGBeatParse(beat_file, infotbl);

			// print KG information
			infotbl->PrintKG();

			if (cfg.storetofile) {
				ls.KGStoreToFile(file_id);
				file_id += 1;
			}

			if (cfg.storeentity) {
				ls.EntityStoreToFile();
			}
		}
	}

	// Graph Visualization (does not support large-scale graphs)
	if (cfg.graph)
		neo4jdb.Neo4jVisKG();

	// store system entities locally
	if (cfg.storeentity || cfg.storetofile)
		ls.DumpProcFileSocketEdge2FactSize(file_id - 1);

	infotbl->FreeInteraction();
	infotbl->FreeNode();
	delete (infotbl);
	return 0;
}
