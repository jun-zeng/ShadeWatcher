#ifndef SHADEWATCHER_PARSER_COMMON_H_
#define SHADEWATCHER_PARSER_COMMON_H_

#include <cinttypes>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>

#ifdef DEBUG
#define db_print(X) std::cerr << X << " " << __FILE__ << " "<<  __LINE__ << std::endl;
#else
#define db_print(X)
#endif

std::ostringstream& operator<<(std::ostringstream&, __uint128_t);

#define uint128tostring(bigint)	({					\
	std::ostringstream _stream;						\
	_stream << bigint;								\
	std::string _str = _stream.str();				\
	_str;											\
})

#define OverheadStart() ({							\
	std::chrono::steady_clock sc;					\
	auto start = sc.now();							\
	start;											\
})

#define OverheadEnd(start, phase) {					\
	std::chrono::steady_clock sc;					\
	double overhead;								\
	auto end = sc.now();							\
	overhead = static_cast<std::chrono::duration<double>>(end - start).count();\
	std::cerr << phase << " runtime overhead: " << overhead << "\n" << std::endl;\
}

#define CHECK_RETURN(call) {						\
	const int error = call;							\
	if (error == 0){								\
		std::cerr << "read log function fails";		\
	}												\
}

#define ProgressBar(width, progress, addup) {		\
 	std::cout << "[";								\
	int pos = width * progress;						\
	for (int i = 0; i < width; ++i) {				\
		if (i < pos) std::cout << "=";				\
		else if (i == pos) std::cout << ">";		\
		else std::cout << " ";						\
	}												\
	std::cout << "] " << int(progress * 100.0 + 0.5) << " %\r" << std::flush;\
	progress += addup;								\
}

typedef __uint128_t uint128_t;
typedef int64_t hash_t;
typedef int64_t sess_t;
typedef int64_t fd_t;
typedef int64_t proc_t;
typedef int64_t file_t;
typedef int64_t nodenum_t; // node_t is occupied by graphviz
typedef int64_t event_t;
typedef int64_t rel_t;
// typedef int64_t seq_t;
typedef uint128_t seq_t;
typedef uint128_t biguint_t;
typedef std::vector<float> embed_t;
typedef int64_t thread_t; 
typedef uint32_t multi_t;

enum class NodeType_t: uint32_t {
	NotDefined = 0,
	Proc,
	File,
	Socket,
	Attr,
};

// First two lines are for syscalls and the rest are for side information
enum class EdgeType_t: uint32_t { 
	NotDefined = 0,
	Vfork,
	Clone,
	Execve,
	Kill,
	Pipe,
	// Delete before Create: differentiate two events in update
	Delete,
	Create,
	Recv,
	Send,
	Mkdir,
	Rmdir,
	Open,
	Load,
	// Read before Write: differentiate two events in rename
	Read,
	Write,
	Connect,
	Getpeername,
	Filepath,
	Mode,
	Mtime,
	Linknum,
	Uid,
	Count,
	Nametype,
	Version,
	Dev,
	SizeByte,
	EdgeType_NR_ITEMS,
};

enum class SyscallType_t: uint32_t{
	NotDefined = 0,
	Vfork,
	Clone,
	Execve,
	Kill,
	Open,
	Pipe,
	Dup,
	Close,
	Read,
	Write,
	Load,
	Connect,
	Delete,
	Create,
	Link,
	Rename,
	Socket,
	Recvfrom,
	Recvmsg,
	Sendto,
	Sendmsg,
	Recv,
	Send,
	Rmdir,
	Mkdir,
	Getpeername,
	Fcntl,
	Update
};

std::string NodeEnum2String(NodeType_t);
std::string EdgeEnum2String(EdgeType_t);
int EdgeEnum2Int(EdgeType_t);
int NodeEnum2Int(NodeType_t);
std::string EdgeInt2String(int);

template <class T> 
void ExchangeMapKey (T &map, hash_t const &id_old, hash_t const &id_new) {
	auto nh = map.extract(id_old);
	// if nh is not empty, nh.empty() = 0
	if (!nh.empty()) {
		nh.key() = id_new;
		map.insert(std::move(nh));
	}
}

std::vector<std::string> split(const std::string&, char);

#endif
