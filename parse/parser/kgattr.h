#ifndef SHADEWATCHER_PARSER_KGATTR_H_
#define SHADEWATCHER_PARSER_KGATTR_H_

#include "kgnode.h"

class NodeAttr: public NodeKG::NodeKG {
public:
	std::string attrtype;
	std::string value;

	NodeAttr(std::string _attrtype, std::string _value);

	void UpdateID();
	void PrintAttr();
};

#endif
