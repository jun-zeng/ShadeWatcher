#ifndef SHADEWATCHER_DB_POSTGRESQL_H_
#define SHADEWATCHER_DB_POSTGRESQL_H_

#include <iostream>
#include <pqxx/pqxx>
#include "parser/kg.h"
#include "util/config.h"
#include <set>

class Postgresqldb
{
public:
	KG *infotbl;

	pqxx::connection *conn;
	int batch_node;
	int batch_edge;
	std::string schema;

	int total_file_num;
	
	// node, proc, file, socket tbls
	int non_edg_tbl_num = 4;

	Postgresqldb(std::string);
	~Postgresqldb();

	void Connectdb(std::string);
	void CreateSchema();
	void CreateNodeTable();
	void CreateEdgeTable(int);

	// form sql queries
	void InsertEmptyNodeSQL(std::string &);
	void InsertSocketSQL(std::string &, std::string &);
	void InsertFileSQL(std::string &, std::string &);
	void InsertProcSQL(std::string &, std::string &);
	void InsertEdgeSQL(std::string &, int);
	void InsertOutNodeSQL(std::string &, std::set<std::string> &);
	void InsertInNodeSQL(std::string &, std::set<std::string> &);

	// insert KG Training data to sql 
	void InsertEntitySQL(std::string &);
	void InsertInteractionSQL(std::string &);
	void InsertRelationSQL(std::string &);
	void InsertKGTrainSQL(std::string &);

	pqxx::result Run(std::string);

	// void CopyFromFile2DBSQL(std::string, std::string, std::string, std::string);

	// insert nodes and edges in KG
	void InsertKGNode(KG *);
	void InsertKGEdge(KG *, int);

	// load entity and edges 
	void LoadProcSQL(pqxx::result &);
	void LoadProcSQL(pqxx::result &, std::string);
	void LoadFileSQL(pqxx::result &);
	void LoadFileSQL(pqxx::result &, std::string);
	void LoadSocketSQL(pqxx::result &);
	void LoadSocketSQL(pqxx::result &, std::string);
	void LoadEdgeSQL(pqxx::result &, int);

	void PrintRes(pqxx::result);

	int CountTblNum();
	int CheckNodeTyp(std::string &);
};

void KGLoadAllFromDB(std::string, KG *, std::string);
void KGLoadPartialFromDB(std::string, KG *, std::string, std::string);
void KGStoreToDB(std::string, KG *, std::string, int);
void KGStoreNodeToDB(std::string, KG *, std::string, int = -1);
void KGStoreEdgeToDB(std::string, KG *, std::string, int = 0);
void KGDeleteSchema(std::string, std::string); 

#endif
