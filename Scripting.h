#ifndef LIS2_SCRIPTING_H
#define LIS2_SCRIPTING_H
#include <string>
#include <lua/lua.hpp>
#include <lua/luawrapper.hpp>
#include <lua/luawrapperutil.hpp>
#include <boost/property_tree/ptree.hpp>
class Lis2DataFrame;
class Lis2FinalView;
class wxDC;
class LIS2DB;
using namespace std;
using boost::property_tree::ptree;
/*
	Script - transparent handling of a lua script as a plugin
*/
class Script
{
public:
	Script(string source);
	~Script();
	void init(string endpointname);
	void readbyte(unsigned int ch);
	void fillDataFrame(Lis2DataFrame* dataframe, ptree* data);
	void fillFinalView(Lis2FinalView* finalview, ptree* data);
	void setParam(ptree* data,string name, string val);
	void drawImage(string draw_func, wxDC* dc, ptree* data);
	void db_create(LIS2DB* lis2db);
	void db_insert(LIS2DB* lis2db, ptree* data);
	string getStringGlobal(string name);
	void order(ptree* order_data, string test_name);
private:	
	lua_State* script;
};


#endif