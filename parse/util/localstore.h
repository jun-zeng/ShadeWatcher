#ifndef SHADEWATCHER_UTIL_LOCALSTORE_H_
#define SHADEWATCHER_UTIL_LOCALSTORE_H_

#include "parser/kg.h"
#include "file.h"

class LocalStore
{
public:
    std::string kg_path;
    KG *infotbl;

    nodenum_t node_num;
    nodenum_t proc_num;
    nodenum_t socket_num;
    nodenum_t file_num;
    event_t edge_num;

    LocalStore(std::string, KG *);

    void StoreProc();
    void StoreFile();
    void StoreSocket();
    void StoreEdge(int);

    // store KG to local files
    void KGStoreToFile(int = 0);

    // store Darpa system entity information
    void EntityStoreToFile();

    // Count the number of procs, files, sockets, and edges
    void DumpProcFileSocketEdge2FactSize(int = 0);
};

// load KG from local files
void KGLoadFromFile(std::string, KG *);

// load Darpa system entity information
void EntityLoadFromFile(std::string, KG *);

#endif
