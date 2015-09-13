#ifndef LIS2_H

#define LIS2_H


// standards section
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <regex>

// boost section
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>

// wxWidgets section
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/file.h>
#include <wx/fs_mem.h>
#include <wx/xrc/xmlres.h>
#include <wx/dcbuffer.h>
// custom wxWidget views for the project
#include <lis2view.h>

// lua headers
#include <lua/lua.hpp>
#include <lua/luawrapper.hpp>
#include <lua/luawrapperutil.hpp>

// custom data model, db, and algorithm headers
#include <models.h>
#include <lisos.h>

#define LIS2_SETUP_FILE "lis2setup.json"
#define LIS2_UNREVIEWED_FILE "lis2unreviewed.json"

class MyApp;
class Lis2Frame;
class Lis2EndpointPage;
class Lis2DataFrame;
class Lis2FinalView;
class CaseSearch;
class SearchResultWindow;

class Lis2Frame : public lis2Frame
{
public :
	Lis2Frame(wxWindow* parent, wxWindowID id = wxID_ANY);
	~Lis2Frame();	
	void closeResultById(int resultId,bool del=false);
	map<int, Lis2DataFrame*> opened_views;
protected :		
	virtual void OnClose(wxCloseEvent& event);
	virtual void fillPatientFromHN(wxFocusEvent& event);
	virtual void fillHNfromPatient(wxFocusEvent& event);
	virtual void formatID(wxFocusEvent& event);
	virtual void OnSubmitOrder(wxCommandEvent& event);
	virtual void OnExitMenuSelected(wxCommandEvent& event);		
	virtual void OnResultItemSelected(wxListEvent& event);
	virtual void OnResultItemDeselected(wxListEvent& event);
	virtual void OnResultKey(wxKeyEvent& event);
	virtual void OnResultView(wxListEvent& event);	
	virtual void OnAddPatientMenu(wxCommandEvent& event);
	virtual void PopUpSearchTool(wxCommandEvent& event);
private :
	bool checkResultOK();
	void endpointAdded(int endpointID);
	void resultSubmitted(int resultID);
	bool LoadXRCFromString(const wxString& memfilename,const wxString& data);
	long resultIdSelected = -1;
	LISSys lis;		
	CaseSearch* search_window = nullptr;
	set<Lis2FinalView*> final_result_windows;
	set<SearchResultWindow*> search_result_windows;
	friend class MyApp;	
	friend class Lis2DataFrame;
	friend class Lis2FinalView;
	friend class CaseSearch;
	friend class SearchResultWindow;
};

class Lis2EndpointPage : public lis2EndpointPage
{
public:
	Lis2EndpointPage(wxWindow* parent, wxWindowID id = wxID_ANY);
	~Lis2EndpointPage();
private:
	friend class Lis2Frame;
};

class Lis2DataFrame : public lis2DataFrame
{
public:
	Lis2DataFrame(wxWindow* parent, Lis2Frame* lf, int rId, wxWindowID id = wxID_ANY, wxString title = "");	
	void clearDataFields();	
	void appendDataField(string param_name,string param_value,string param_default, string param_unit);
	void insertDataField(int row, string param_name, string param_value, string param_default, string param_unit);	
	void clearImageFields();
	void addImagePage(string page_name, string draw_func_name);
	void renderDrawPanel(wxDC& dc);	
	void putComment(string comment);
private:
	map<string, string> draw_func;
	string current_draw_func;
	int resultId;
	Script* getScript();
	ResultDef* getResult();
	virtual void ActivateParamEditor(wxListEvent& event);
	virtual void FormatID(wxFocusEvent& event);
	virtual void FormatHN(wxFocusEvent& event);
	virtual void OnClose(wxCloseEvent& event);
	virtual void OnImageChosen(wxCommandEvent& event);
	virtual void OnPaintImage(wxPaintEvent& event);
	virtual void OnDeleteResult(wxCommandEvent& event);
	virtual void OnCloseView(wxCommandEvent& event);
	virtual void OnRevertResult(wxCommandEvent& event);
	virtual void OnApplyResult(wxCommandEvent& event);
	virtual void OnSendToDB(wxCommandEvent& event);
	virtual void setComment(wxFocusEvent& event);
	Lis2Frame* lis2frame;
	friend class Lis2Frame;
	bool editmode;		
	ptree originalResult;
};

class Lis2FinalView : public lis2FinalView
{
public:
	Lis2FinalView(wxWindow *parent, Lis2Frame* lf,FinalDef& f, wxWindowID id=wxID_ANY);
	void clearDataFields();
	void appendDataField(string param_name, string param_value, string param_default, string param_unit);
	void insertDataField(int row, string param_name, string param_value, string param_default, string param_unit);
	void clearImageFields();
	void addImagePage(string page_name, string draw_func_name);
	void renderDrawPanel(wxDC& dc);
	void putComment(string comment);
private:
	void renderChoice();
	map<string, string> draw_func;
	map<string, Script*> scripts;
	virtual void OnClose(wxCloseEvent& event);
	virtual void dataChoiceChanged(wxCommandEvent& event);
	virtual void OnImageChosen(wxCommandEvent& event);
	virtual void OnPaintImage(wxPaintEvent& event);
	virtual void OnCloseBtn(wxCommandEvent& event);
	virtual void OnExport(wxCommandEvent& event);
	FinalDef final_def;
	Lis2Frame* lis2frame;
	friend class Lis2Frame;
};

class ParamEditDialog : public paramEditDialog
{
public:
	enum Type { NUM, ALNUM, ALPHA };
	ParamEditDialog(wxWindow* parent, wxWindowID id = wxID_ANY);
	string value();
	void value(wstring v);
	bool valueChanged();
	void set(wstring name, wstring original, wstring unit, Type t);	
	void reset();	
private:	
	virtual void resetParam(wxCommandEvent& event);
	virtual void confirmParam(wxCommandEvent& event);		
	wstring paramOriginal;
	string paramConfirmedVal;
	Type type;	
};
class PatientDialog : public patientDialog
{
public:
	PatientDialog(wxWindow* parent, LISSys* l, wxWindowID id = wxID_ANY);
private:
	LISSys *lis;
	virtual void formatHN(wxFocusEvent& event);
	virtual void onOKBtn(wxCommandEvent& event);
	virtual void onCancelBtn(wxCommandEvent& event);	
	bool submitted;
	PatientInfoDef info;
	friend class Lis2Frame;
};

class PatientConflictDialog : public patientConflictDialog
{
public: 
	PatientConflictDialog(wxWindow* parent, wxWindowID id = wxID_ANY);
private:
	virtual void onYesBtn(wxCommandEvent& event);
	virtual void onNoBtn(wxCommandEvent& event);
	friend class Lis2Frame;
	friend class PatientDialog;
	bool conflict_confirmed;
};

class CaseSearch : public caseSearchFrame
{
public:
	CaseSearch(wxWindow* parent, Lis2Frame* lf, wxWindowID id = wxID_ANY);
private:
	virtual void OnIDCheck(wxCommandEvent& event);
	virtual void FormatID(wxFocusEvent& event);
	virtual void OnHNCheck(wxCommandEvent& event);
	virtual void FormatHN(wxFocusEvent& event);
	virtual void OnNameCheck(wxCommandEvent& event);
	virtual void OnFromCheck(wxCommandEvent& event);
	virtual void OnToCheck(wxCommandEvent& event);
	virtual void OnSearch(wxCommandEvent& event);
	virtual void OnClose(wxCloseEvent& event);
	friend class Lis2Frame;
	Lis2Frame* lis2frame;
};
struct SearchOption
{
public:
	bool _id, _hn, _name, _from, _to;
	int id, hn;
	string first_name;
	string last_name;
	wxDateTime from;
	wxDateTime to;
};
class SearchResultWindow : public searchResultWindow
{
public:
	SearchResultWindow(wxWindow* parent, Lis2Frame* lf,vector<CaseDef>& r, SearchOption& opt,wxWindowID id = wxID_ANY);
private:
	void FormatSearchOptions(SearchOption& opt);
	void push_result(const CaseDef& result);
	vector<CaseDef> results;
	Lis2Frame* lis2frame;
	virtual void ResultChosen(wxListEvent& event);	
	virtual void OnClose(wxCloseEvent& event);
	friend class Lis2Frame;
};

class MyApp : public wxApp
{
public:
	Lis2Frame *frame;
	virtual bool OnInit();
};

#endif /* LIS2_H */