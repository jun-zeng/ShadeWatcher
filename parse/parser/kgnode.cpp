#include "kgnode.h"

NodeKG::NodeKG(NodeType_t _type) {
	id = (hash_t *) malloc (sizeof(hash_t));
	type = _type;
}

NodeKG::~NodeKG() {
	free (id);
}
