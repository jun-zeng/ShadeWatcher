#include "reduce_noise.h"

ReduceNoise::ReduceNoise(KG *_infotbl) {
	infotbl = _infotbl;
}

ReduceNoise::~ReduceNoise() {
}

bool ReduceNoise::is_create(EdgeType_t rel) {
	return (rel == EdgeType_t::Create);
}

bool ReduceNoise::is_delete(EdgeType_t rel) {
	return (rel == EdgeType_t::Delete);
}

bool ReduceNoise::is_file_or_socket(NodeType_t n_typ) {
	return (n_typ == NodeType_t::File || n_typ == NodeType_t::Socket);
}

bool ReduceNoise::is_write_or_send(EdgeType_t rel) {
	return (rel == EdgeType_t::Write || rel == EdgeType_t::Send);
}

bool ReduceNoise::is_read_or_recv(EdgeType_t rel) {
	return (rel == EdgeType_t::Read || rel == EdgeType_t::Recv);
}

void ReduceNoise::insertNoisyevents(std::vector <hash_t> NoiseEdgeTable) { 
	for (auto e_id: NoiseEdgeTable) {
		noisy_edges.insert(e_id);
	}
}

bool CompareInter(KGEdge *e1, KGEdge *e2) {
	if (e1->seq != e2->seq) {
		return (e1->seq < e2->seq);
	}
	else {
		return (EdgeEnum2Int(e1->relation) < EdgeEnum2Int(e2->relation));
	}
}

void ReduceNoise::TmpFile() {
	interaction_map intertbl = infotbl->FileInteractionTable;

	// Traverse the file nodes stored in file_interaction_map
	for (auto file: intertbl){
		hash_t f_id = file.first;
		inter_map *interactions = file.second;
		// sort interactions accroding to sequence
		sort(interactions->begin(), interactions->end(), CompareInter);

		bool temporary_file_flag = false;
		std::vector <hash_t> NoiseEdgeTable;

		auto iter = interactions->begin();
		KGEdge* edge = *iter;

		// A temporary file must be a newly created file
		if (!is_create(edge->relation)) {
			continue;
		}
		hash_t n1_hash = edge->n1_hash;
		hash_t parent_id = n1_hash;
		NoiseEdgeTable.push_back(edge->e_id);

		for (iter++; iter != interactions->end(); iter++) {
			KGEdge* edge = *iter;
			hash_t n1_hash = edge->n1_hash;

			hash_t p_id;
			if (n1_hash == f_id) {
				p_id = edge->n2_hash;
			}
			else {
				p_id = n1_hash;
			}

			if (p_id == parent_id) {
				NoiseEdgeTable.push_back(edge->e_id);
			}
			else {
				// A file accessed by different processes is not a temporary file
				break;
			}
			
			if (is_delete(edge->relation)) {
				temporary_file_flag = true;
				// Normally, this break does not work! 
				// The delete syscall should be the last operations for this file
				break;
			}
		}

		// delete all operations regarding temporary files
		if (temporary_file_flag == true) {
			insertNoisyevents(NoiseEdgeTable);
		}
	}
}

void ReduceNoise::ShadowFileEdge() {
	interaction_map intertbl = infotbl->FileInteractionTable;
	std::vector <hash_t> NoiseEdgeTable;

	// Traverse file nodes stored in FileInteractionTable
	for (auto file: intertbl) {
		// sort interactions accroding to sequence
		inter_map *interactions = file.second;
		sort(interactions->begin(), interactions->end(), CompareInter);

		// Traverse proc interactions in inter_map
		for (auto iter = interactions->rbegin(); iter != interactions->rend(); iter++){
			KGEdge* edge = *iter;
			EdgeType_t rel = edge->relation;
			hash_t n1_hash = edge->n1_hash;
			hash_t n2_hash = edge->n2_hash;

			bool potential_not_redundant_flag = false;
			if (is_write_or_send(rel)) {
				// n1 is a proc and n2 is a file
				auto backward_iter = std::next(iter, 1);
				for (; backward_iter != interactions->rend(); backward_iter++) {
					// Only do backward searching
					KGEdge* new_edge = *backward_iter;
					hash_t new_n1_hash = new_edge->n1_hash;

					// If the file is written or sent by the same process later,
					// we consider this edge as shadow event
					if (n1_hash == new_n1_hash) {
						EdgeType_t new_rel = new_edge->relation;
						
						if (EdgeEnum2String(new_rel) == "notdefined") {
							std::cout << "Sequence " << uint128tostring(edge->seq) << std::endl;
						}
						if (is_write_or_send(new_rel)) {
							break;
						}
					}
					// If the file is accessed (read, receive...) later,
					// we consider this edge as key events
					else if (n2_hash == new_n1_hash) {
						potential_not_redundant_flag = true;
						break;
					}
					// Last interaction do not trigger break
					if (backward_iter == std::make_reverse_iterator(interactions->begin()))
						potential_not_redundant_flag = true;
				}

				if (potential_not_redundant_flag == false && backward_iter != interactions->rend()) {
					NoiseEdgeTable.push_back(edge->e_id);
				}
			}
		}
	}

	// delete all shadow events
	insertNoisyevents(NoiseEdgeTable);
}

void ReduceNoise::ShadowProcEdge() {
	interaction_map intertbl = infotbl->ProcInteractionTable;
	std::vector <hash_t> NoiseEdgeTable;
	
	// Traverse proc nodes stored in ProcInteractionTable
	for (auto proc: intertbl) {
		inter_map *interactions = proc.second;

		// sort interactions accroding to sequence
		sort(interactions->begin(), interactions->end(), CompareInter);

		// Traverse file interactions in inter_map
		for (auto iter = interactions->rbegin(); iter != interactions->rend(); iter++){
			KGEdge* edge = *iter;
			EdgeType_t rel = edge->relation;
			hash_t n1_hash = edge->n1_hash;
			hash_t n2_hash = edge->n2_hash;

			bool potential_not_redundant_flag = false;
			if (is_read_or_recv(rel)) {
				// n1 is a file and n2 is a proc
				auto backward_iter = std::next(iter, 1);
				for (; backward_iter != interactions->rend(); backward_iter++) {
					// Only do backward searching
					KGEdge* new_edge = *backward_iter;
					hash_t new_n1_hash = new_edge->n1_hash;

					// If the proc read or receive the same file later,
					// we consider this edge as shadow events
					if (n1_hash == new_n1_hash) {
						EdgeType_t new_rel = new_edge->relation;
						if (is_read_or_recv(new_rel)) {
							break;
						}
					}
					// If we find this proc accesses system entities later,
					// we consider this edge as key events
					else if (n2_hash == new_n1_hash) {
						potential_not_redundant_flag = true;
						break;
					}
					// Last interaction still not triggers break
					if (backward_iter == std::make_reverse_iterator(interactions->begin())) 
						potential_not_redundant_flag = true;
				}

				if (potential_not_redundant_flag == false && backward_iter != interactions->rend()) {
					NoiseEdgeTable.push_back(edge->e_id);
				}
			}
		}
	}

	// delete all shadow events
	insertNoisyevents(NoiseEdgeTable);
}

void ReduceNoise::MissingEdge() {
	edge_map &em = infotbl->KGEdgeTable;
	for (auto it: em) {
		KGEdge* edge = it.second;
		hash_t n1_hash = edge->n1_hash;
		hash_t n2_hash = edge->n2_hash;
		bool n1_not_exist = infotbl->SearchNodeNotExist(n1_hash);
		bool n2_not_exist = infotbl->SearchNodeNotExist(n2_hash);

		// we delete an edge if one of its node cannot be found in KGNodeTable
		if (n1_not_exist || n2_not_exist) {
			noisy_edges.insert(edge->e_id);
		}
	}
}

void ReduceNoise::Library() {
	interaction_map p_intertbl = infotbl->ProcInteractionTable;
	std::vector <hash_t> NoiseEdgeTable;
	std::unordered_set<hash_t> libraries;

	// Traverse proc stored in interaction_map
	for (auto proc: p_intertbl) {
		inter_map *interactions = proc.second;
		for (auto edge: *interactions) {
			// Libraries are in load edges belonging to execve syscalls
			if (edge->relation != EdgeType_t::Load) {
				continue;
			}

			// identify libraries; Not every file loaded in execve syscall is a library
			hash_t f_hash = edge->n1_hash;
			hash_t p_hash = edge->n2_hash;
			auto f_it = libraries.find(f_hash);
			if (f_it != libraries.end()) {
				NoiseEdgeTable.push_back(edge->e_id);
			}
			else if (TrackabilityCheck(f_hash, p_hash, 1)) {
				NoiseEdgeTable.push_back(edge->e_id);
				libraries.insert(f_hash);
			}
		}
	}
	
	// delete all mundane events
	insertNoisyevents(NoiseEdgeTable);
}

// direction: 0 == out; 1 == in
bool ReduceNoise::TrackabilityCheck(hash_t f_hash, hash_t p_hash, int direction) {
	NodeType_t n_type = infotbl->SearchNodeType(f_hash);
	if (n_type != NodeType_t::File) {
		return false;
	}

	interaction_map f_intertbl = infotbl->FileInteractionTable;
	auto file = f_intertbl.find(f_hash);
	if (file == f_intertbl.end()) {
		std::cerr << "file " << f_hash << " donot have interactions " << __FILE__ << " "<<  __LINE__ << std::endl;
		exit(EXIT_FAILURE);
	}

	inter_map *f_interactions = file->second;
	if (direction == 1) {
		for (auto edge: *f_interactions) {
			if (f_hash == edge->n2_hash && p_hash != edge->n1_hash) {
				return false;
			}
		}
	}
	else {
		for (auto edge: *f_interactions) {
			if (f_hash == edge->n1_hash && p_hash != edge->n2_hash) {
				return false;
			}
		}
	}
	return true;
}

int ReduceNoise::time_window(std::string &ts_s, std::string &ts_e) {
	if (infotbl->audit_source == auditbeat) {
		// format: 2020-10-31T14:14:47.925Z
		int sec_s = std::stoi(ts_s.substr(17, 18));
		int sec_e = std::stoi(ts_e.substr(17, 18));
		return (sec_e >= sec_s) ? (sec_e - sec_s) : (sec_e - sec_s + 60);
	}
	else if (infotbl->audit_source == darpa) {
		// format: 1523631648049000000 nanosecond
		auto sec_s = std::stoll(ts_s.substr(0, ts_s.size() - 9));
		auto sec_e = std::stoll(ts_e.substr(0, ts_e.size() - 9));
		return (sec_e - sec_s);
	}
	else {
		std::cerr << "Cannot identify audit source " << __FILE__ << " "<<  __LINE__ << std::endl;
		exit(EXIT_FAILURE);
	}
}

void ReduceNoise::DeleteNoise() {
	auto start = OverheadStart();
	std::cout << "Reduce noisy events" << std::endl;

	// collect noisy events
	std::cout << "\tcollecting temporary file" << std::endl;
	TmpFile();
	std::cout << "\tcollecting ShadowFileEdge" << std::endl;
	ShadowFileEdge();
	std::cout << "\tcollecting ShadowProcEdge" << std::endl;
	ShadowProcEdge();
	std::cout << "\tcollecting MissingEdge" << std::endl;
	MissingEdge();
	std::cout << "\tcollecting Library" << std::endl;
	Library();

	// remove noisy events
	std::cout << "\tdeleting nosiy events" << std::endl;
	edge_map &em = infotbl->KGEdgeTable;
	for (auto e_id: noisy_edges) {
		auto nh_edge = em.extract(e_id);
		if (empty(nh_edge) == false) {
			KGEdge* s = nh_edge.mapped();
			delete(s);
			infotbl->noise_num += 1;
		}
	}
	OverheadEnd(start, "Reduce Noise Events");
}
