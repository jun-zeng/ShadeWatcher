#include "common.h"

std::ostringstream& operator<<(std::ostringstream& dest, __uint128_t value)
{
    std::ostringstream::sentry s(dest);
    if (s) {
        __uint128_t tmp = value;
        char buffer[128];
        char* d = std::end(buffer);
        do {
            -- d;
            *d = "0123456789"[tmp % 10];
            tmp /= 10;
        } while (tmp != 0);
        int len = std::end(buffer) - d;
        if (dest.rdbuf()->sputn(d, len) != len) {
            dest.setstate(std::ios_base::badbit);
        }
    }
    return dest;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}

std::string NodeEnum2String(NodeType_t type) {
	switch(type){
		case NodeType_t::Proc:
			return "proc";
		case NodeType_t::File:
			return "file";
		case NodeType_t::Socket:
			return "socket";
		case NodeType_t::Attr:
			return "attr";
		case NodeType_t::NotDefined:
			std::cerr << "undefined node " << __FILE__ << " "<<  __LINE__ << std::endl;
			return "notdefined";
	}
	return "";
}

int NodeEnum2Int(NodeType_t type) {
	switch(type){
		case NodeType_t::Proc:
			return 1;
		case NodeType_t::File:
			return 2;
		case NodeType_t::Socket:
			return 3;
		case NodeType_t::Attr:
			return 4;
		case NodeType_t::NotDefined:
			std::cerr << "undefined node " << __FILE__ << " "<<  __LINE__ << std::endl;
			return 0;
	}
	return 0;
}

std::string EdgeEnum2String(EdgeType_t type) {
	switch(type){
		case EdgeType_t::Vfork:
			return "vfork";
		case EdgeType_t::Clone:
			return "clone";
		case EdgeType_t::Execve:
			return "execve";
		case EdgeType_t::Kill:
			return "kill";
		case EdgeType_t::Create:
			return "create";
		case EdgeType_t::Pipe:
			return "pipe";
		case EdgeType_t::Delete:
			return "delete";
		case EdgeType_t::Recv:
			return "recv";
		case EdgeType_t::Send:
			return "send";
		case EdgeType_t::Mkdir:
			return "mkdir";
		case EdgeType_t::Rmdir:
			return "rmdir";
		case EdgeType_t::Open:
			return "open";
		case EdgeType_t::Load:
			return "load";
		case EdgeType_t::Read:
			return "read";
		case EdgeType_t::Write:
			return "write";
		case EdgeType_t::Connect:
			return "connect";
		case EdgeType_t::Getpeername:
			return "getpeername";
		case EdgeType_t::Filepath:
			return "filepath";
		case EdgeType_t::Mode:
			return "mode";
		case EdgeType_t::Mtime:
			return "mtime";
		case EdgeType_t::Linknum:
			return "linknum";
		case EdgeType_t::Uid:
			return "uid";
		case EdgeType_t::Count:
			return "count";
		case EdgeType_t::Nametype:
			return "nametype";
		case EdgeType_t::Version:
			return "version";
		case EdgeType_t::Dev:
			return "dev";
		case EdgeType_t::SizeByte:
			return "sizebyte";
		case EdgeType_t::EdgeType_NR_ITEMS:
			return "edgetype_num";
		case EdgeType_t::NotDefined:
			std::cerr << "undefined relation " << __FILE__ << " "<<  __LINE__ << std::endl;
			return "notdefined";
	}
	return "";
}

int EdgeEnum2Int(EdgeType_t type) {
	switch(type){
		case EdgeType_t::Vfork:
			return 1;
		case EdgeType_t::Clone:
			return 2;
		case EdgeType_t::Execve:
			return 3;
		case EdgeType_t::Kill:
			return 4;
		case EdgeType_t::Pipe:
			return 5;
		case EdgeType_t::Delete:
			return 6;
		case EdgeType_t::Create:
			return 7;
		case EdgeType_t::Recv:
			return 8;
		case EdgeType_t::Send:
			return 9;
		case EdgeType_t::Mkdir:
			return 10;
		case EdgeType_t::Rmdir:
			return 11;
		case EdgeType_t::Open:
			return 12;
		case EdgeType_t::Load:
			return 13;
		case EdgeType_t::Read:
			return 14;
		case EdgeType_t::Write:
			return 15;
		case EdgeType_t::Connect:
			return 16;
		case EdgeType_t::Getpeername:
			return 17;
		case EdgeType_t::Filepath:
			return 18;
		case EdgeType_t::Mode:
			return 19;
		case EdgeType_t::Mtime:
			return 20;
		case EdgeType_t::Linknum:
			return 21;
		case EdgeType_t::Uid:
			return 22;
		case EdgeType_t::Count:
			return 23;
		case EdgeType_t::Nametype:
			return 24;
		case EdgeType_t::Version:
			return 25;
		case EdgeType_t::Dev:
			return 26;
		case EdgeType_t::SizeByte:
			return 27;
		case EdgeType_t::EdgeType_NR_ITEMS:
			return 28;
		case EdgeType_t::NotDefined:
			std::cerr << "undefined relation " << __FILE__ << " "<<  __LINE__ << std::endl;
			return 0;
	}
	return 0;
}

std::string EdgeInt2String(int id) {
	switch(id){
		case 1:
			return "vfork";
		case 2:
			return "clone";
		case 3:
			return "execve";
		case 4:
			return "kill";
		case 5:
			return "create";
		case 6:
			return "pipe";
		case 7:
			return "delete";
		case 8:
			return "recv";
		case 9:
			return "send";
		case 10:
			return "mkdir";
		case 11:
			return "rmdir";
		case 12:
			return "open";
		case 13:
			return "load";
		case 14:
			return "read";
		case 15:
			return "write";
		case 16:
			return "connect";
		case 17:
			return "getpeername";
		case 18:
			return "filepath";
		case 19:
			return "mode";
		case 20:
			return "mtime";
		case 21:
			return "linknum";
		case 22:
			return "uid";
		case 23:
			return "count";
		case 24:
			return "nametype";
		case 25:
			return "version";
		case 26:
			return "dev";
		case 27:
			return "sizebyte";
		case 28:
			return "edgetype_num";
		case 0:
			std::cerr << "undefined relation " << __FILE__ << " "<<  __LINE__ << std::endl;
			return "";
	}
	return "";
}
