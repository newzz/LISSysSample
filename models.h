#ifndef LIS2_MODELS_H
#define LIS2_MODELS_H

#define LIS2_MAIN_DB "LIS2.db"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <Scripting.h>
#include <LIS2DB.h>

#define CBC 1
#include <aes.h>

#define LISERROR -1
using namespace std;
using boost::property_tree::ptree;
using boost::asio::io_service;
using boost::asio::serial_port;
using boost::asio::serial_port_base;
using boost::thread;
using boost::thread_group;

#define BPARITY_NONE serial_port_base::parity::type::none
#define BPARITY_ODD serial_port_base::parity::type::odd
#define BPARITY_EVEN serial_port_base::parity::type::even

#define BSTOPBITS_1 serial_port_base::stop_bits::type::one
#define BSTOPBITS_1_5 serial_port_base::stop_bits::type::onepointfive
#define BSTOPBITS_2 serial_port_base::stop_bits::type::two

#define BFLOWCTL_NONE serial_port_base::flow_control::type::none
#define BFLOWCTL_SOFT serial_port_base::flow_control::type::software
#define BFLOWCTL_HARD serial_port_base::flow_control::type::hardware

#define BITS_PER_BYTE 8

// standalone helper
class IDGen;
// model declarations
struct EndpointDef;
struct ProtocolDef;
struct OrderDef;
struct ResultDef;
typedef boost::function<void(int)> int_callback;
/*
//struct FilterDef;
//struct DBDef;
*/

// combined system
class LISSys;

class IDGen
{
public:
	IDGen();
	~IDGen();
	int operator()();
	void operator()(int const reuse);
	void reset( int const num = 0);
private:
	int id;
	queue<int> reuse_pool;
};

struct EndpointDef
{
public:
	EndpointDef();
	~EndpointDef();
	int id;
	string name;
	string deviceName;	
	string protocolName;
	int protocolId;;
	enum State { CLOSED, OPENED } state;
	serial_port* serial;
	Script* script;
	unsigned char chbuf;		
};

struct ProtocolDef
{
public:
	ProtocolDef();
	~ProtocolDef();
	int id = -1;
	string name;
	bool canOrder;
	string filename;
	string scriptsrc;
	string db_location;
	string db_alias;
	enum Type { T_SERIAL, T_NET } type;
	unordered_set<string> tests;
	struct SerialOption
	{
	public:
		SerialOption();
		serial_port_base::parity::type parity;
		serial_port_base::stop_bits::type stopbit;
		serial_port_base::flow_control::type flowctrl;
		unsigned int baudrate;
		unsigned int databits;
	} serial_option;
};

struct ResultDef
{
public:
	ResultDef();
	~ResultDef();
	int id;
	time_t time;
	string device, protocol, time_s,desc;
	enum Status { PENDING, PARTIAL, COMPLETE } status;
	int endpointId;
	int protocolId;
	ptree data;
};

struct FinalDef
{
public:
	FinalDef();
	~FinalDef();
	CaseDef case_def;
	map<string, ptree> tests_data;
};

class thread_notifier {
public:
	thread_notifier() : cond(), mut(), var(false) {}
	void wait_until_true()
	{
		boost::unique_lock<boost::mutex> lock(mut);
		while (!var)
		{
			cond.wait(lock);
		}
	}
	void setVar(bool val)
	{
		{
			boost::lock_guard<boost::mutex> lock(mut);
			var = val;
		}
	}
	void setVarNotify(bool val)
	{
		{
			boost::lock_guard<boost::mutex> lock(mut);
			var = val;
		}
		cond.notify_one();
	}
private:
	boost::condition_variable cond;
	boost::mutex mut;
	bool var;
};
class LISSys
{
public:	
	static LISSys* getInstance();
	LISSys();
	~LISSys();
	void loadSetting(string filename);
	void loadResults(string filename);
	void openEndpoints();
	int loadEndpoint(ptree& def);
	int loadProtocol(ptree& def);	
	int findProtocolID(string name);
	int findEndpointID(string name);	
	// TODO: allow synchronization with the GUI part
	void setOnEndpointAdded(int_callback cbk);
	void setOnEndpointRemoved(int_callback cbk); // TODO endpoint might be removed ??
	vector<string> getEndpointNames();
	vector<string> getProtocolNames();
	map<int,ResultDef>& getResults();
	set<string> getAvailableTests();
	void order(ptree& order_data, vector<CaseDef>& old_cases);
	bool removeResult(int resultId);	
	void setOnResultSubmitted(int_callback cbk); // not here ?
	void setOnResultApproved(int_callback cbk); // not here ?
	void setOnResultViewed(int_callback cbk); // not here ?
	void submit(ptree& pt); // submit to results section
	// edit(int resultId, ptree newdata);
	//* approve(int resultId) //
	EndpointDef* findEndpoint(int id);
	ProtocolDef* findProtocol(int id);	
	ResultDef* findResult(int id);
	bool isEndpointOpen(int endpointID);
	bool openEndpoint(int endpointID);	
	LIS2DB* getDB();
protected:
	bool serial_port_avail(string device);
	bool assignProtocolID(EndpointDef& endpoint);
	void read_hndl(int endpointID, const boost::system::error_code& error,
		std::size_t bytes_transferred);	
private:	
	int_callback endpointAdded=0, 
		endpointRemoved=0, 
		resultSubmitted=0,
		resultApproved=0, 
		resultViewed=0;
	void io_thread_worker();
	IDGen endpointIdGen, protocolIdGen, resultIdGen;
	map<string, int> endpointAlias, protocolAlias;
	map<int, EndpointDef> endpoints;
	map<int, ProtocolDef> protocols;	
	unordered_set<int> viewedResults;
	map<int, ResultDef> results;
	LIS2DB db;
	io_service io;	
	thread_notifier noti;
	thread_group threadGroup;
	io_service::strand strand_;
	string aes_key;
	bool sys_ended;		
};

void print_tree(ptree const& pt, int level = 0);

#endif