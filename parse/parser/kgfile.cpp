#include "kgfile.h"

NodeFile::NodeFile(std::string _name, std::string _version, hash_t _hash): NodeKG(NodeType_t::File) {
	name = _name;
	version = _version; 
	*id = _hash; 
}

NodeFile::NodeFile(std::string _name, std::string _version, std::string _hash): NodeKG(NodeType_t::File) {
	name = _name;
	version = _version; // should be a space when used in darpa dataset

	std::hash<std::string> hasher;
	// std::string str = name + version;
	*id = (hash_t)hasher(_hash); 
}

NodeFile::NodeFile(std::string _name, hash_t _id): NodeKG(NodeType_t::File) {
	name = _name;
	*id = _id;
}

NodeFile::NodeFile(std::string _name): NodeKG(NodeType_t::File) {
	name = _name;
	version = " ";

	// hash(name + version) ==> id
	std::hash<std::string> hasher;
	std::string str = name + version;
	*id = (hash_t)hasher(str); 
}

NodeFile::NodeFile(std::string _name, std::string _version): NodeKG(NodeType_t::File) {
	name = _name;
	version = _version;

	// hash(name + version) ==> id
	std::hash<std::string> hasher;
	std::string str = name + version;
	*id = (hash_t)hasher(str); 
}

hash_t NodeFile::ComputeID(std::string _name, std::string _version){
	std::hash<std::string> hasher;
	std::string str = _name + _version;
	return (hash_t)hasher(str);
}

void NodeFile::UpdateID() {
	std::hash<std::string> hasher;
	std::string str = name + version;
	*id = (hash_t)hasher(str);  
}

void NodeFile::PrintFile() {
	std::cout << "name " << name;
	std::cout << "\tversion " << version << std::endl;
}
