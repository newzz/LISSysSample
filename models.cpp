#include <models.h>

IDGen::IDGen() : id(0){}
IDGen::~IDGen(){}
void IDGen::operator()(int const reuse) 
{  
	reuse_pool.push(reuse);
}

int IDGen::operator()()
{ 
	if (!reuse_pool.empty())
	{
		int reuse = reuse_pool.front();
		reuse_pool.pop();
		return reuse;
	}
	return id++;  
}
void IDGen::reset(int const num)
{
	id = num;
	while (!reuse_pool.empty()) reuse_pool.pop();
}
EndpointDef::EndpointDef() : id(-1), protocolId(-1), serial(nullptr), script(nullptr), state(CLOSED){}
EndpointDef::~EndpointDef(){}
ProtocolDef::ProtocolDef() :id(-1), type(T_SERIAL){}
ProtocolDef::~ProtocolDef(){}
ProtocolDef::SerialOption::SerialOption() : parity(BPARITY_NONE), stopbit(BSTOPBITS_1), flowctrl(BFLOWCTL_NONE), baudrate(9600), databits(8){}

ResultDef::ResultDef() :id(-1), protocolId(-1), status(PENDING)
{
}
ResultDef::~ResultDef() 
{
}

int LISSys::findProtocolID(string name)
{
	if (protocolAlias.count(name)) return protocolAlias[name];
	return -1;
}
int LISSys::findEndpointID(string name)
{
	if (endpointAlias.count(name)) return endpointAlias[name];
	return -1;
}

void LISSys::loadSetting(string filename)
{
	ifstream jsonis(filename);	
	ptree pt;
	boost::property_tree::read_json(jsonis, pt);
	auto& endpointsSetting = pt.get_child("endpoints");
	auto& protocolsSetting = pt.get_child("protocols");
	auto& pEnd = protocolsSetting.end();
	auto& eEnd = endpointsSetting.end();
	for (auto pIt = protocolsSetting.begin(); pIt!=pEnd; ++pIt)
	{
		auto& protocol_ = pIt->second;
		loadProtocol(protocol_);
	}
	for (auto eIt = endpointsSetting.begin(); eIt != eEnd; ++eIt)
	{
		auto& endpoint_ = eIt->second;
		loadEndpoint(endpoint_);
	}
}
void LISSys::loadResults(string filename)
{
	ifstream jsonis(filename);
	ptree results_j;
	try 
	{
		boost::property_tree::read_json(jsonis, results_j);
	}
	catch (...)
	{

	}

	for (auto rIt = results_j.begin(), rEnd = results_j.end(); rIt != rEnd; ++rIt)
	{		
		auto& data = rIt->second;
		submit(data);
	}
}

void LISSys::openEndpoints()
{
	for (auto& pair : endpoints)
	{
		openEndpoint(pair.first);
	}
}
int LISSys::loadEndpoint(ptree& def)
{
	int id = endpointIdGen();
	auto& endpoint = endpoints[id];
	endpoint.id = id;
	endpoint.name = def.get<string>("name", "");
	endpoint.deviceName = def.get<string>("device", "");
	endpoint.protocolName = def.get<string>("protocol", "");	
	assignProtocolID(endpoint);
	endpointAlias[endpoint.name] = id;
	if (!endpointAdded.empty()) endpointAdded(id);
	return id;
}
bool LISSys::assignProtocolID(EndpointDef& endpoint)
{
	int& id = endpoint.protocolId;
	if (id != -1) return true;
	id = findProtocolID(endpoint.protocolName);
	return id != -1;
}
int LISSys::loadProtocol(ptree& def)
{
	int id = protocolIdGen();
	auto& protocol = protocols[id];
	float tmpI;
	string tmp;
	protocol.id = id;
	protocol.name = def.get<string>("name", "");
	
	tmp = def.get<string>("type", "serial");		
	protocol.type = tmp == "serial" ? ProtocolDef::Type::T_SERIAL : ProtocolDef::Type::T_NET;
	protocol.filename = def.get<string>("filename", "");
	protocol.canOrder = def.get<bool>("canOrder", true);
	protocol.scriptsrc = "";
	protocol.db_location = def.get<string>("db_location", "");
	protocol.db_alias = def.get<string>("db_alias", "");
	auto& tests = def.get_child("tests");
	for (auto testIt = tests.begin(), testEnd = tests.end(); testIt != testEnd; ++testIt)
	{
		auto test = testIt->second.get_value<string>();
		protocol.tests.insert(test);
	}
	if (tmp != "")
	{		
		ifstream ifs(protocol.filename);
		wstringstream ss;
		if (ifs.good()){
			protocol.scriptsrc.assign((istreambuf_iterator<char>(ifs)),
				(istreambuf_iterator<char>()));
			ss << protocol.filename.c_str() << " read." << endl;
		}
		else
		{			
			ss << protocol.filename.c_str() << " could not be read." << endl;			
		}
		OutputDebugString(ss.str().c_str());
	}

	switch (protocol.type)
	{
	case ProtocolDef::Type::T_SERIAL:

		tmp = def.get<string>("options.parity", "none");
		protocol.serial_option.parity =	tmp == "even" ? BPARITY_EVEN :	tmp == "odd" ? BPARITY_ODD : BPARITY_NONE;

		tmpI = def.get<float>("options.stopbit", 1.f );
		protocol.serial_option.stopbit = tmpI == 2.f ? BSTOPBITS_2 : tmpI == 1.5f ? BSTOPBITS_1_5 : BSTOPBITS_1;

		tmp = def.get<string>("options.flowctrl", "none");
		protocol.serial_option.flowctrl = tmp == "hardware" ? BFLOWCTL_HARD : tmp == "software" ? BFLOWCTL_SOFT : BFLOWCTL_NONE;

		protocol.serial_option.baudrate = def.get<unsigned int>("options.baudrate", 9600);
		protocol.serial_option.databits = def.get<unsigned int>("options.databits", 8);
		OutputDebugString(L"T_SERIAL protocol loaded\n");
		break;
	case ProtocolDef::Type::T_NET:
		OutputDebugString(L"T_NET protocol type currently unsupported\n");
		protocolIdGen(id);
		return LISERROR;
	}

	protocolAlias[protocol.name] = id;
	return id;
}
EndpointDef* LISSys::findEndpoint(int id)
{
	if (endpoints.count(id))
	{
		auto& endpoint = endpoints[id];
		assignProtocolID(endpoint);
		return &endpoint;
	}
	return nullptr;
}
ProtocolDef* LISSys::findProtocol(int id)
{
	if (protocols.count(id)) return &protocols[id];
	return nullptr;
}

ResultDef* LISSys::findResult(int id)
{
	if (results.count(id)) return &results[id];
	return nullptr;
}
bool LISSys::isEndpointOpen(int endpointID)
{
	EndpointDef * endpoint = findEndpoint(endpointID);
	if (endpoint){
		return endpoint->state == EndpointDef::OPENED;
	}
	else return false;
}

bool LISSys::openEndpoint(int endpointID)
{
	auto endpoint = findEndpoint(endpointID);
	if (endpoint){
		if (endpoint->state == EndpointDef::OPENED)
		{
			return false;
		}
		else
		{
			// open port...
			auto protocol = findProtocol(endpoint->protocolId);
			if (protocol){
				switch (protocol->type){
				case ProtocolDef::T_SERIAL:
					if (!serial_port_avail(endpoint->deviceName)) return false;
					endpoint->serial = new serial_port{ io, endpoint->deviceName };
					if (protocol->scriptsrc != "")
					{
						endpoint->script = new Script(protocol->scriptsrc);						
						endpoint->script->init(endpoint->name);
						
					}					
					else
					{
						endpoint->script = nullptr;
					}
					endpoint->serial->set_option(serial_port_base::baud_rate(protocol->serial_option.baudrate));
					endpoint->serial->set_option(serial_port_base::parity(protocol->serial_option.parity));
					endpoint->serial->set_option(serial_port_base::character_size(protocol->serial_option.databits));
					endpoint->serial->set_option(serial_port_base::stop_bits(protocol->serial_option.stopbit));
					endpoint->serial->set_option(serial_port_base::flow_control(protocol->serial_option.flowctrl));
					endpoint->state = EndpointDef::OPENED;
					endpoint->serial->async_read_some(boost::asio::buffer(&endpoint->chbuf, 1), 
						strand_.wrap(boost::bind(&LISSys::read_hndl, this, endpoint->id, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred))
					);
					db.attach(protocol->db_location, protocol->db_alias);
					endpoint->script->db_create(&db);
					noti.setVarNotify(true);
					break;
				default:
					// unsupported protocol type
					return false;
				}
				return true;
			}
			else
			{				
				wstringstream ss;
				ss << "ERROR: protocol " << endpoint->protocolName.c_str() << " not found" << endl;
				OutputDebugString(ss.str().c_str());
				cout << "ERROR: protocol " << endpoint->protocolName.c_str() << " not found" << endl;
				return false;
			}
		}
	}
	return false;
}

void LISSys::setOnEndpointAdded(int_callback cbk)
{
	endpointAdded = cbk;
}
void LISSys::setOnEndpointRemoved(int_callback cbk)
{
	endpointRemoved = cbk;
}
vector<string> LISSys::getEndpointNames()
{
	vector<string> result;
	for (auto& pair : endpointAlias)
		result.push_back(pair.first);	
	return result;
}
vector<string> LISSys::getProtocolNames()
{
	vector<string> result;
	for (auto& pair : protocolAlias)
		result.push_back(pair.first);	
	return result;
}
set<string> LISSys::getAvailableTests()
{
	set<string> result;
	for (auto& pair : endpoints)
	{
		auto& endpoint = pair.second;
		auto protocol = findProtocol(endpoint.protocolId);
		for (auto& test : protocol->tests)
		{
			result.insert(test);
		}
	}		
	return result;
}
map<int,ResultDef>& LISSys::getResults()
{
	return results;
}

void LISSys::order(ptree& order_data, vector<CaseDef>& old_cases)
{
	vector<string> tests;
	boost::algorithm::split(tests, order_data.get("tests", ""), boost::is_any_of(","));
	for (auto& pair : endpoints)
	{
		auto& endpoint = pair.second;
		auto protocol = findProtocol(endpoint.protocolId);
		for (auto& protocol_test : protocol->tests)
		{
			for (auto& test_ordered : tests)
			{
				if (test_ordered == protocol_test) 
				{
					if (old_cases.empty())
					{
						endpoint.script->order(&order_data, test_ordered);
					}
					else
					{
						auto& old_case = old_cases[0];
						bool found = false;
						for (auto& old_test : old_case.tests)
						{
							found |= (test_ordered == old_test);
							if (found) break;
						}
						if (!found)
						{
							endpoint.script->order(&order_data, test_ordered);
						}
					}
				}
			}
		}			
	}	
}
void LISSys::setOnResultSubmitted(int_callback cbk)
{
	resultSubmitted = cbk;
}
void LISSys::setOnResultApproved(int_callback cbk)
{
	resultApproved = cbk;
}
void LISSys::setOnResultViewed(int_callback cbk)
{
	resultViewed = cbk;
}

void LISSys::read_hndl(int endpointID, const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (!error)
	{
		auto endpoint = findEndpoint(endpointID);
		if (endpoint->script)
		{			
			endpoint->script->readbyte(endpoint->chbuf);			
		}
		else
		{
			// default bahavior is to print through debug output						
			wstringstream ss;
			ss << "endpoint " << endpointID << ": " << int(endpoint->chbuf) << endl;
			OutputDebugString(ss.str().c_str());
		}
		endpoint->serial->async_read_some(boost::asio::buffer(&endpoint->chbuf, 1),
			strand_.wrap(boost::bind(&LISSys::read_hndl, this, endpoint->id, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred))
			);
		noti.setVarNotify(true);
	}	
}

bool LISSys::removeResult(int resultId)
{
	if (results.count(resultId))
	{
		results.erase(resultId);
		resultIdGen(resultId);
		return true;
	}
	return false;
}
void LISSys::submit(ptree& pt)
{	
	auto id = resultIdGen();
	auto& result = results[id];
	result.time = pt.get<long long>("time",chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());	
	result.id = id;	
	result.data = pt;
	result.device = pt.get("device", "");
	result.protocol = pt.get("protocol", "");
	result.endpointId = findEndpointID(result.device);
	result.protocolId = findProtocolID(result.protocol);		
	result.time_s = pt.get("time_s", "n/a");
	result.desc = pt.get("desc", "");
	if (!resultSubmitted.empty()) resultSubmitted(id);	
}

bool LISSys::serial_port_avail(string device)
{	
	boost::asio::serial_port ser{ io };
	bool ret = true;
	try
	{
		ser.open(device);
		ser.close();
	}
	catch (exception&)
	{
		ret = false;
	}
	return ret;
}

void LISSys::io_thread_worker()
{	
	while (!sys_ended){
		noti.wait_until_true();		
		noti.setVar(false);
		io.run_one();
	}	
	io.run();	
}
static LISSys* _lastLISInstance;
LISSys::LISSys() : io(), sys_ended(false), strand_(io), db(LIS2_MAIN_DB, [](wstring s) { OutputDebugString(s.c_str()); }), aes_key("InfocusLIS2 2015")
{	
	threadGroup.create_thread(boost::bind(&LISSys::io_thread_worker, this));
	_lastLISInstance = this;
}
LISSys* LISSys::getInstance()
{
	return _lastLISInstance;
}
LIS2DB* LISSys::getDB()
{
	return &db;
}

LISSys::~LISSys()
{
	for (auto& pair : endpoints)
	{
		auto& endpoint = pair.second;
		auto serial = endpoint.serial;
		auto script = endpoint.script;
		if (serial)
		{
			// free serial
			serial->close();
			io.post([serial](){
				delete serial;				
			});
		}
		if (script)
		{
			delete script;
			script = nullptr;
		}
	}
	sys_ended = true;
	noti.setVarNotify(true);
	threadGroup.join_all();
}

void print_tree(ptree const& pt, int level)
{
	for (auto it = pt.begin(), end = pt.end(); it != end; ++it) {
		wstringstream ss;
		for (int i = 0; i < level; i++) ss << "-";
		ss << "|" << it->first.c_str() << ": " << it->second.get_value<string>().c_str() << endl;
		OutputDebugString(ss.str().c_str());
		print_tree(it->second, level + 1);
	}
}

FinalDef::FinalDef() : hn(0),id(0) {  }
FinalDef::~FinalDef() {}