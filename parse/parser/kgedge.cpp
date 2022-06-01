#include "kgedge.h"

// for auditbeat
KGEdge::KGEdge(hash_t *n1, hash_t *n2, EdgeType_t r, seq_t seq_, sess_t sess_, std::string timestamp_) {
	e_id = 0;
	n1_id = n1;
	n2_id = n2;
	relation = r;
	seq = seq_;
	sess = sess_;
	timestamp = timestamp_;
}

// for darpa / loading auditbeat data
KGEdge::KGEdge(hash_t n1, hash_t n2, EdgeType_t r, seq_t seq_, sess_t sess_, hash_t uuid, std::string timestamp_) {
	e_id = uuid;
	n1_hash = n1;
	n2_hash = n2;
	relation = r;
	seq = seq_;
	sess = sess_;
	timestamp = timestamp_;
} 
