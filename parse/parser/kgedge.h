#ifndef SHADEWATCHER_PARSER_KGEDGE_H_
#define SHADEWATCHER_PARSER_KGEDGE_H_

#include "common.h"

class KGEdge {
public:
	hash_t e_id;
	hash_t *n1_id;
	hash_t *n2_id;
	EdgeType_t relation;
	seq_t seq;
	sess_t sess;
	hash_t n1_hash;
	hash_t n2_hash;
	std::string timestamp;

	KGEdge(hash_t *, hash_t *, EdgeType_t, seq_t, sess_t, std::string);
	KGEdge(hash_t, hash_t, EdgeType_t, seq_t, sess_t, hash_t, std::string);
};

#endif
