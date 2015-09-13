#ifndef LIS2DB_H
#define LIS2DB_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/function.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sqlite3/sqlite3.h>
using namespace std;
using boost::property_tree::ptree;

struct PatientInfoDef
{
public:
	int hn;
	string first_name;
	string last_name;
};
struct CaseDef
{
public:	
	int uid;
	int id;
	int hn;
	int date;
	double time;
	vector<string> tests;
};
struct Blob
{
public:
	Blob(int cap);
	~Blob();
	uint8_t* get();
	uint8_t get(int idx);
	void set(int idx, uint8_t val);
	void clear();
private:
	int capacity;
	uint8_t* raw;
};
struct ResultCallbackDef
{
public:
	boost::function<int(sqlite3_stmt*)> process_row = 0;
};
class LIS2DB
{
public:
	LIS2DB(string filename, boost::function<void(wstring)> debugPrinter=0);
	~LIS2DB();		
	// database-wide stuff
	bool register_prepared_statement(string key, string stmt);
	sqlite3_stmt* get_prepared_statement(string key);
	void attach(string db_location, string alias);
	void exec(string stmt, ResultCallbackDef cbk);
	// patient CRUD
	bool patient_exists(int hn);
	bool insert_patient(PatientInfoDef& info);	
	PatientInfoDef getPatientInfo(int hn);// def.hn < 0 if does not exist
	PatientInfoDef getPatientInfo(string firstname, string lastname);
	bool delete_patient(int hn);	
	// case CRUD
	bool insert_case(CaseDef& case_info);
	vector<CaseDef> getCases(string first,string last,double begin_datetime, double end_datetime);
	vector<CaseDef> getCases(int hn, double begin_datetime, double end_datetime);
	vector<CaseDef> getCasesByID(int id, double begin_datetime, double end_datetime);
	vector<CaseDef> getCasesByIDHN(int id, int hn, double begin_datetime, double end_datetime);
	CaseDef getCasesByUID(int uid);
	/*
	void detach(alias);		
	//*/
	void print(wstring s);
	void setPrinter(boost::function<void(wstring)> p);
private:	
	void database_check();
	void prepare_statements();
	boost::function<void(wstring)> printer;
	char* msgbuf;
	bool table_exists(string table_name);
	sqlite3 *db;
	sqlite3_stmt *db_attacher;
	map<string, sqlite3_stmt*> prepared_statements;
	bool opened;
};

#endif