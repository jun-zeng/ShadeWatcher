#include "kgsocket.h"

NodeSocket::NodeSocket(std::string _name, std::string _uuid): NodeKG(NodeType_t::Socket) {
	name = _name;

	// internel socket: hash("socket:[4941446]")" == > id
	// externel socket: hash(ip + ":" + port) ==> id
	std::hash<std::string> hasher;
	*id = (hash_t)hasher(_uuid); 
}

NodeSocket::NodeSocket(std::string _name, hash_t _id): NodeKG(NodeType_t::Socket) {
	name = _name;
	*id = _id;
}

NodeSocket::NodeSocket(std::string _name): NodeKG(NodeType_t::Socket) {
	name = _name;

	// internel socket: hash("socket:[4941446]")" == > id
	// externel socket: hash(ip + ":" + port) ==> id
	std::hash<std::string> hasher;
	*id = (hash_t)hasher(name); 
}

void NodeSocket::UpdateID(std::string _new_name) {
	name = _new_name;

	std::hash<std::string> hasher;
	*id = (hash_t)hasher(name);
}

void NodeSocket::PrintSocket() {
	std::cout << "name " << name << std::endl;
}
