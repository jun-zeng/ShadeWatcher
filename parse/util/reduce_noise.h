#ifndef SHADEWATCHER_UTIL_REDUCENOISE_H_
#define SHADEWATCHER_UTIL_REDUCENOISE_H_

#include "parser/kg.h"
#include "parser/common.h"
#include <set>
#include <unordered_set>

// proc hash => hash of <seq + rel + fname>
typedef std::unordered_map<hash_t, std::vector<KGEdge *> *> action_edges_map;
typedef std::unordered_map<std::string, int> action_seq_map;
typedef std::unordered_map<hash_t, action_seq_map *, HashFunction> proc_seq_map;

class ReduceNoise {
public:
	KG *infotbl;
	std::set<hash_t> noisy_edges;

	ReduceNoise(KG *);
	~ReduceNoise();
	
	bool is_file_or_socket(NodeType_t);
	bool is_write_or_send(EdgeType_t);
	bool is_create(EdgeType_t);
	bool is_delete(EdgeType_t);
	bool is_read_or_recv(EdgeType_t);
	int time_window(std::string &, std::string &);
	bool TrackabilityCheck(hash_t, hash_t, int);

	void insertNoisyevents(std::vector <hash_t>);

	// Different noise reduction strategies
	void TmpFile();
	void ShadowFileEdge();
	void ShadowProcEdge();
	void Library();
	void MissingEdge();

	// delete edges from edge_map
	void DeleteNoise();
};

#endif
