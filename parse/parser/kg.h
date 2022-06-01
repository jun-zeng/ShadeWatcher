#ifndef SHADEWATCHER_PARSER_KG_H_
#define SHADEWATCHER_PARSER_KG_H_

// define audit source
#define auditbeat 0
#define darpa 1

#include "kgproc.h"
#include "kgfile.h"
#include "kgsocket.h"
#include "kgattr.h"
#include "kgedge.h"
#include "readlog.h"
#include <dirent.h>
#include <set>

// Overload hash function in unordered_map
struct HashFunction {
	hash_t operator()(const hash_t key) const { 
		return (hash_t) key;
	}
};

// we overload hash function in unordered_map. 
// The hast_t ID is used for key
typedef std::unordered_map<std::string, NodeProc *> internal_pid_map;
typedef std::list <KGEdge *> internal_edge_map;
typedef std::unordered_map <hash_t, NodeProc *, HashFunction> proc_map;
typedef std::unordered_map <hash_t, NodeFile *, HashFunction> file_map;
typedef std::unordered_map <hash_t, NodeSocket *, HashFunction> socket_map;
typedef std::unordered_map <hash_t, NodeAttr *, HashFunction> attr_map;
typedef std::unordered_map <hash_t, NodeType_t, HashFunction> node_map;
typedef std::unordered_map<hash_t, std::vector<hash_t> *, HashFunction> internal_proc_fd_map; 
typedef std::unordered_map<hash_t, KGEdge *, HashFunction> edge_map;
typedef std::vector <KGEdge *> inter_map;
typedef std::unordered_map <hash_t,  inter_map *, HashFunction> interaction_map;

class KG {
public:
	// 0:auditbeat 1:darpa
	int audit_source = -1;

	event_t event_num = 0;
	event_t edge_num = 0;
	event_t noise_num = 0;

	// three tables store info to different node types (proc, file and attr)
	proc_map ProcNodeTable;
	file_map FileNodeTable;
	socket_map SocketNodeTable;
	attr_map AttrNodeTable;

	// this table stores nodes in a graph
	node_map KGNodeTable;

	// this talbe stores edges in a graph
	edge_map KGEdgeTable;

	// this table stores noisy nodes (e.g., firefox process) in darpa datasets
	node_map NoiseTable;

	// this table stores interactions for each file and socket
	// used to reduce noise
	interaction_map FileInteractionTable;

	// this table stores interactions for each proc
	interaction_map ProcInteractionTable;

	// internal use for auditd and auditbeat file access syscall
	internal_proc_fd_map ProcFdMap;

	// internal use for searchProc(pid);
	internal_pid_map PidTable;

	// this list stores edges in a graph
	// we dont use unordered_map to store edges in the first place 
	// because we dont want to update edge key constantly 
	internal_edge_map KGEdgeList;

	// List files and procs that may trigger dependency explosino problem
	std::vector <std::string> NoiseFile;
	std::vector <std::string> NoiseProc;

	KG(int);
	~KG();

	// Load
	bool LoadProc(std::string);
	bool LoadFd(std::string);
	bool LoadSocket(std::string);

	// Delete
	bool DeleteFd(hash_t, std::string);

	// Insert
	void InsertNoisyNode(hash_t, NodeType_t);
	void InsertNode(hash_t , NodeType_t);
	NodeProc* InsertProc(NodeProc *);
	bool InsertLoginProc(const Json::Value, LogLoader *, event_t);
	bool InsertLoginProc(const Json::Value, std::map<seq_t, Json::Value>, std::map<seq_t, Json::Value>::iterator);
	void InsertPid(std::string, NodeProc *);
	NodeFile* InsertFile(NodeFile *);
	NodeSocket* InsertSocket(NodeSocket *);
	NodeAttr* InsertAttr(NodeAttr *);
	void InsertEdge(KGEdge *);
	void InsertEdge(hash_t, KGEdge *);
	bool InsertFd(hash_t, std::string, hash_t);
	void InsertEmptyFd(hash_t);
	bool CopyFd(hash_t, hash_t);
	bool CopyFd(hash_t, std::string, std::string);
	void InsertFileInteraction(hash_t, KGEdge *);
	void InsertProcInteraction(hash_t, KGEdge *);
	void InsertFileInteraction(hash_t, inter_map *);
	void InsertProcInteraction(hash_t, inter_map *);

	// search
	bool SearchNoisyNode(hash_t);
	NodeType_t SearchNodeType(hash_t);
	std::pair<NodeType_t, std::string> SearchNode(hash_t);
	bool SearchNodeNotExist(hash_t);
	NodeProc* SearchProc(hash_t);
	NodeProc* SearchProc(std::string);
	NodeFile* SearchFile(hash_t);
	NodeSocket* SearchSocket(hash_t);
	NodeSocket* SearchSocket(std::string);
	NodeAttr* SearchAttr(hash_t);
	std::vector<hash_t>* SearchFd(hash_t);

	// the last step
	void ProcInfoRecover();
	void GenKGEdgeTable();
	void GenKGEdgeTableWhenLoad();
	void DeletePipeEdge();
	void DeleteLocalSocket();

	// Print
	void PrintNodeTable();
	void PrintProcTable();
	void PrintPidTable();
	void PrintFileTable();
	void PrintSocketTable();
	void PrintAttrTable();
	void PrintEdgeTable();
	void PrintEdges(std::vector<KGEdge *>);
	void PrintFd();
	void PrintFd(hash_t);
	void PrintKG();
	void PrintFileInteraction();
	void PrintProcInteraction();

	// desconstruct objects in a KG to free memory
	void FreeInteraction();
	void FreeNode();
};

#endif
