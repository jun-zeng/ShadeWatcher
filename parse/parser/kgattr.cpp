#include "kgattr.h"

NodeAttr::NodeAttr(std::string _attrtype, std::string _value): NodeKG(NodeType_t::Attr) {
	attrtype = _attrtype;
	value = _value;
	
	// hash(attrtype, value) ==> id
	std::hash<std::string> hasher;
	std::string str = attrtype + value;
	*id = (hash_t)hasher(str);
}

void NodeAttr::UpdateID() {
	std::hash<std::string> hasher;
	std::string str = value + attrtype;
	*id = (hash_t)hasher(str);
}

void NodeAttr::PrintAttr() {
	std::cout << "attrtype " << attrtype;
	std::cout << "\tvalue " << value << std::endl;
}
