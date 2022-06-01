#include "driverdar.h"

int main(int argc, char **argv) {
	// parse command line arguments
	Config cfg(argc, argv);
	cfg.ConfigDar();

	// Knowledge graph for darpa
	KG *infotbl = new KG(darpa);
	
	// define Local File in KG (no computation)
	LocalStore ls(cfg.embed_data_path, infotbl);

	// visulize in neo4j
	Neo4jdb neo4jdb(cfg.neo4j_config, infotbl);

	// KG Construction
	// 1. load kg from db/file 
	// 2. parse logs to construct kg
	if (cfg.loadentity) {
		// load system entities from local files
		EntityLoadFromFile(cfg.embed_data_path, infotbl);
		// print KG information
		infotbl->PrintKG();
	}

	if (cfg.loadfromdb) {
		// load knowledge graph from database
		if (cfg.ld_all_edg_flag == true) {
			KGLoadAllFromDB(cfg.postgres_config, infotbl, cfg.darpa_dataset);
		}
		else {
			KGLoadPartialFromDB(cfg.postgres_config, infotbl, cfg.darpa_dataset, cfg.edg_ld_files);
		}
		// print KG information
		infotbl->PrintKG();
	}
	else if (cfg.loadfromfile) {
		// load knowledge graph from local files
		KGLoadFromFile(cfg.embed_data_path, infotbl);
		// print KG information
		infotbl->PrintKG();
	}
	else {
		// collect darpa files under darpa_data_dir
		auto darpa_files = CollectJsonFile(cfg.darpa_data_dir);

		int file_id = 0;
		for (auto darpa_file: darpa_files) {
			// construct KG (work_threads shows #threads to parse dataset)
			KGConstruction(darpa_file, infotbl, cfg);

			// print KG information
			infotbl->PrintKG();

			// store kg in local file or database
			// TODO: specify the file id to insert edge.
			if(cfg.storetodb) {
				KGStoreToDB(cfg.postgres_config, infotbl, cfg.darpa_dataset, file_id);
				file_id += 1;
			}
			
			if (cfg.storetofile) {
				ls.KGStoreToFile(file_id);
				file_id += 1;
			}
			
			if (cfg.storeentity) {
				ls.EntityStoreToFile();
			}

			// ************ visualize attacks in darpa dataset *******************
			// ******************************************************************

			// to save memory, we clean up KG after parsing a darpa file
			infotbl->FreeInteraction();
		}

		if (cfg.storetofile || cfg.storeentity) {
			ls.DumpProcFileSocketEdge2FactSize(file_id - 1);
		}
	}

	infotbl->FreeInteraction();
	infotbl->FreeNode();
	delete (infotbl);
	return 0;
}
