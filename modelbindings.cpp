#include <modelbindings.h>

// library bindings 
int luaopen_ScriptLibs(lua_State* L)
{
	luaL_openlibs(L);	
	luaopen_hpdf(L);
	lua_register(L, "pdebug", lPDebug);
	luaW_register<ptree>(L, "ptree",lPtree_table, lPtree_meta, lPtree_new);
	luaW_register<LISSys>(L, "LISSys",lLISSys_table, lLISSys_meta, lLISSys_new);
	luaW_register<Lis2DataFrame>(L,"Lis2DataFrame",lLis2DF_table,lLis2DF_meta,NULL);
	luaW_register<wxDC>(L, "wxDC", lwxDC_table, lwxDC_meta, NULL);
	luaW_register<LIS2DB>(L, "LIS2DB", lLIS2DB_table, lLIS2DB_meta, NULL);
	luaW_register<sqlite3_stmt>(L, "sqlite3_stmt", lStmt_table, lStmt_meta, NULL);
	luaW_register<Blob>(L, "LisBlob", lBlob_table, lBlob_meta, NULL);
	return 1;
}

int lPDebug(lua_State* L)
{
	auto message = luaL_checkstring(L, 1);
	wstringstream ss;
	ss << message;
	OutputDebugString(ss.str().c_str());
	return 0;
}
map<string, ptree> ptreePool;
int lPtree_sGet(lua_State* L)
{
	auto name = luaL_checkstring(L, 1);	
	luaW_push<ptree>(L, &ptreePool[name]);
	return 1;
}
ptree* lPtree_new(lua_State* L)
{
	return new ptree;
}
int lPtree_clear(lua_State* L)
{
	auto pt = luaW_check<ptree>(L, 1);
	pt->clear();
	return 0;
}
int lPtree_has(lua_State*L)
{
	auto pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	lua_pushboolean(L, bool(pt->get_child_optional(key)) );
	return 1;
}
int lPtree_getProp(lua_State* L)
{
	auto pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = pt->get<string>(key, "");
	lua_pushstring(L, val.c_str());
	return 1;
}
int lPtree_setProp(lua_State* L)
{
	ptree* pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = luaL_checkstring(L, 3);
	pt->put(key, val);
	return 0;
}
int lPtree_getBool(lua_State* L)
{
	ptree* pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = pt->get<bool>(key, false);
	lua_pushboolean(L, val);
	return 1;
}
int lPtree_setBool(lua_State* L)
{
	ptree* pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = lua_toboolean(L, 3);
	pt->put(key, val);
	return 0;
}
int lPtree_getNum(lua_State* L)
{
	ptree* pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = pt->get<float>(key, 0.f);
	lua_pushnumber(L, val);
	return 1;
}
int lPtree_setNum(lua_State* L)
{
	ptree* pt = luaW_check<ptree>(L, 1);
	auto key = luaL_checkstring(L, 2);
	auto val = luaL_checknumber(L, 3);
	pt->put(key, val);
	return 0;
}
LISSys* lLISSys_new(lua_State *L)
{
	return new LISSys;
}
int lLISSys_getLast(lua_State *L)
{
	luaW_push<LISSys>(L, LISSys::getInstance());
	return 1;
}
int lLISSys_Submit(lua_State* L)
{
	auto lis = luaW_check<LISSys>(L, 1);
	auto pt = luaW_check<ptree>(L, 2);
	lis->submit(*pt);
	return 0;
}

int lLis2DF_setTitle(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	auto title = luaL_checkstring(L, 2);
	dataframe->SetTitle(title);
	return 0;
}

int lLis2DF_clearDataFields(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	dataframe->clearDataFields();
	return 0;
}

int lLis2DF_appendDataField(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	auto param_name = luaL_checkstring(L, 2);
	auto param_value = luaL_checkstring(L, 3);
	auto param_default = luaL_checkstring(L, 4);
	auto param_unit = luaL_checkstring(L, 5);
	dataframe->appendDataField(param_name,param_value,param_default,param_unit);
	return 0;
}
int lLis2DF_insertDataField(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	int row = luaL_checknumber(L, 2);
	auto param_name = luaL_checkstring(L, 3);
	auto param_value = luaL_checkstring(L, 4);
	auto param_default = luaL_checkstring(L, 5);
	auto param_unit = luaL_checkstring(L, 6);
	dataframe->insertDataField(row,param_name, param_value, param_default, param_unit);
	return 0;
}
int lLis2DF_clearImages(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	dataframe->clearImageFields();
	return 0;
}

int lLis2DF_addImage(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	auto tab_name = luaL_checkstring(L, 2);
	auto draw_func_name = luaL_checkstring(L, 3);
	dataframe->addImagePage(tab_name, draw_func_name);
	return 0;
}

int lLis2DF_comment(lua_State* L)
{
	auto dataframe = luaW_check<Lis2DataFrame>(L, 1);
	auto comment = luaL_checkstring(L, 2);
	dataframe->putComment(comment);
	return 0;
}

int lwxDC_brush(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	unsigned long colorcode = luaL_checknumber(L, 2);
	int brushsize = luaL_checknumber(L, 3);	
	dc->SetBrush(brushsize ? wxBrush(wxColor(colorcode),brushsize) : *wxTRANSPARENT_BRUSH);
	return 0;
}
int lwxDC_pen(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	unsigned long colorcode = luaL_checknumber(L, 2);
	int pensize = luaL_checknumber(L, 3);
	dc->SetPen(pensize ? wxPen(wxColor(colorcode), pensize) : *wxTRANSPARENT_PEN);
	return 0;
}
int lwxDC_line(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	auto x1 = luaL_checknumber(L, 2);
	auto y1 = luaL_checknumber(L, 3);
	auto x2 = luaL_checknumber(L, 4);
	auto y2 = luaL_checknumber(L, 5);
	dc->DrawLine(wxPoint(x1,y1),wxPoint(x2,y2));
	return 0;
}

int lwxDC_rect(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	auto x = luaL_checknumber(L, 2);
	auto y = luaL_checknumber(L, 3);
	auto w = luaL_checknumber(L, 4);
	auto h = luaL_checknumber(L, 5);
	dc->DrawRectangle(wxPoint(x, y), wxSize(w, h));
	return 0;
}

int lwxDC_text(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	auto text = luaL_checkstring(L, 2);
	auto x = luaL_checknumber(L, 3);
	auto y = luaL_checknumber(L, 4);
	dc->DrawText(text, wxPoint(x,y));
	return 0;
}

int lwxDC_size(lua_State* L)
{
	auto dc = luaW_check<wxDC>(L, 1);
	int w, h;
	dc->GetSize(&w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int lLIS2DB_exec(lua_State* L)
{
	lua_settop(L, 3);	
	auto lis2db = luaW_check<LIS2DB>(L, 1);
	auto stmt = luaL_checkstring(L, 2);	
	auto callback_name = lua_isnil(L, 3)? nullptr : luaL_checkstring(L, 3);
	ResultCallbackDef cbk;
	if (callback_name)
	{
		cbk.process_row = [=](sqlite3_stmt* pstmt) {
			lua_getglobal(L, callback_name);
			luaW_push<sqlite3_stmt>(L, pstmt);
			lua_pcall(L, 1, 0, 0);
			return 0;
		};
	}
	lis2db->exec(stmt, cbk);
	return 0;
}

int lLIS2DB_prepare(lua_State* L)
{
	auto lis2db = luaW_check<LIS2DB>(L, 1);
	auto name = luaL_checkstring(L, 2);
	auto stmt = luaL_checkstring(L, 3);
	lis2db->register_prepared_statement(name, stmt);
	return 0;
}

int lLIS2DB_getStmt(lua_State* L)
{
	auto lis2db = luaW_check<LIS2DB>(L, 1);
	auto name = luaL_checkstring(L, 2);
	luaW_push<sqlite3_stmt>(L, lis2db->get_prepared_statement(name));
	return 1;
}

int lStmt_indexOf(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	auto name = luaL_checkstring(L, 2);
	lua_pushinteger(L,sqlite3_bind_parameter_index(stmt, name));
	return 1;
}

int lStmt_step(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	lua_pushboolean(L, sqlite3_step(stmt)==SQLITE_ROW);
	return 1;
}

int lStmt_stepAll(lua_State* L)
{
	lua_settop(L, 2);
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	auto callback_name = lua_isnil(L, 2) ? nullptr : luaL_checkstring(L, 2);	
	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		if (callback_name)
		{
			lua_getglobal(L, callback_name);
			luaW_push<sqlite3_stmt>(L, stmt);
			lua_pcall(L, 1, 0, 0);			
		}
	}
	return 0;
}

int lStmt_reset(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	sqlite3_reset(stmt);
	return 0;
}

int lStmt_bindInt(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	int val = luaL_checkinteger(L, 3);
	sqlite3_bind_int(stmt, idx, val);
	return 0;
}

int lStmt_bindNum(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	auto val = luaL_checknumber(L, 3);
	sqlite3_bind_double(stmt, idx, val);
	return 0;
}

int lStmt_bindStr(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	auto val = luaL_checkstring(L, 3);
	sqlite3_bind_text(stmt, idx, val, -1, SQLITE_TRANSIENT );
	return 0;
}

int lStmt_bindBlob(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	auto blob = luaW_check<Blob>(L, 3);
	int len = luaL_checkinteger(L, 4);
	sqlite3_bind_blob(stmt, idx, blob->get(), len, SQLITE_TRANSIENT);
	return 0;
}

int lStmt_colInt(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_pushinteger(L,sqlite3_column_int(stmt, idx));
	return 1;
}

int lStmt_colNum(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_pushnumber(L, sqlite3_column_double(stmt, idx));
	return 1;
}

int lStmt_colStr(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_pushstring(L, reinterpret_cast<const char*>(sqlite3_column_text(stmt,idx)));
	return 1;
}

int lStmt_colBlob(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	int len = luaL_checkinteger(L, 3);
	auto blob_raw = sqlite3_column_blob(stmt, idx);
	auto blob = new Blob(len);
	memcpy(blob->get(), blob_raw, len);
	luaW_push<Blob>(L, blob);
	return 1;
}

int lStmt_isNull(lua_State* L)
{
	auto stmt = luaW_check<sqlite3_stmt>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_pushboolean(L, sqlite3_column_type(stmt,idx)==SQLITE_NULL);
	return 1;
}

int lBlob_alloc(lua_State* L)
{
	int cap = luaL_checkinteger(L, 1);
	luaW_push<Blob>(L, new Blob(cap));
	return 1;
}

int lBlob_free(lua_State* L)
{
	auto blob = luaW_check<Blob>(L, 1);
	delete blob;
	return 0;
}

int lBlob_get(lua_State* L)
{
	auto blob = luaW_check<Blob>(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_pushnumber(L, blob->get(idx));
	return 1;
}
int lBlob_set(lua_State* L)
{
	auto blob = luaW_check<Blob>(L, 1);
	int idx = luaL_checkinteger(L, 2);	
	uint8_t val = uint8_t(luaL_checkinteger(L, 3));
	blob->set(idx, val);
	return 0;
}
int lBlob_clear(lua_State* L)
{
	auto blob = luaW_check<Blob>(L, 1);
	blob->clear();
	return 0;
}