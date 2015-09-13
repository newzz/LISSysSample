#ifndef LIS2_MODEL_BINDINGS_H
#define LIS2_MODEL_BINDINGS_H
#include <models.h>
#include <LIS2.h>
#include <luahpdf.h>

int lPDebug(lua_State* L);

int lPtree_sGet(lua_State* L);
ptree* lPtree_new(lua_State* L);
int lPtree_getProp(lua_State* L);
int lPtree_setProp(lua_State* L);
int lPtree_getBool(lua_State* L);
int lPtree_setBool(lua_State* L);
int lPtree_getNum(lua_State* L);
int lPtree_setNum(lua_State* L);
int lPtree_clear(lua_State*L);
int lPtree_has(lua_State*L);

static luaL_Reg lPtree_table[] = {
	{ "get", lPtree_sGet },
	{ NULL, NULL }
};

static luaL_Reg lPtree_meta[] = {
	{ "getProp", lPtree_getProp },
	{ "setProp", lPtree_setProp },
	{ "getBool", lPtree_getBool },
	{ "setBool", lPtree_setBool },
	{ "getNum", lPtree_getNum },
	{ "setNum", lPtree_setNum },
	{"clear",lPtree_clear},
	{"has",lPtree_has},
	{ NULL, NULL }
};

LISSys* lLISSys_new(lua_State *L);
int lLISSys_getLast(lua_State *L);
int lLISSys_Submit(lua_State* L);

static luaL_Reg lLISSys_table[] = {
	{ "get", lLISSys_getLast },
	{ NULL, NULL }
};
static luaL_Reg lLISSys_meta[] = {
	{ "submit", lLISSys_Submit },
	{ NULL, NULL }
};


int lLis2DF_setTitle(lua_State*L);
int lLis2DF_clearDataFields(lua_State*L);
int lLis2DF_appendDataField(lua_State*L);
int lLis2DF_insertDataField(lua_State*L);
int lLis2DF_clearImages(lua_State* L);
int lLis2DF_addImage(lua_State* L);
int lLis2DF_comment(lua_State* L);

static luaL_Reg lLis2DF_table[] = {
	{NULL,NULL}
};
static luaL_Reg lLis2DF_meta[] = {
	{ "setTitle", lLis2DF_setTitle},
	{ "clearDataFields", lLis2DF_clearDataFields},
	{ "insertDataField", lLis2DF_insertDataField },
	{ "appendDataField", lLis2DF_appendDataField },
	{ "clearImages" , lLis2DF_clearImages },
	{ "addImage" , lLis2DF_addImage },
	{ "comment" , lLis2DF_comment },
	{NULL,NULL}
};
int lwxDC_brush(lua_State* L);
int lwxDC_pen(lua_State* L);
int lwxDC_line(lua_State* L);
int lwxDC_rect(lua_State* L);
int lwxDC_text(lua_State* L);
int lwxDC_size(lua_State* L);

static luaL_Reg lwxDC_table[] = {
	{ NULL,NULL }
};
static luaL_Reg lwxDC_meta[] = {	
	{ "brush", lwxDC_brush },
	{ "pen", lwxDC_pen },
	{ "line", lwxDC_line },	
	{ "rect", lwxDC_rect },
	{ "text", lwxDC_text},
	{ "size",lwxDC_size},
	{ NULL,NULL }
};

int lLIS2DB_exec(lua_State* L);
int lLIS2DB_prepare(lua_State* L);
int lLIS2DB_getStmt(lua_State* L);

static luaL_Reg lLIS2DB_table[] = {
	{ NULL,NULL }
};
static luaL_Reg lLIS2DB_meta[] = {
	{ "exec", lLIS2DB_exec },	
	{ "prepare", lLIS2DB_prepare },
	{ "getStmt", lLIS2DB_getStmt },
	{ NULL,NULL }
};

int lStmt_indexOf(lua_State* L);
int lStmt_step(lua_State* L);
int lStmt_stepAll(lua_State* L);
int lStmt_reset(lua_State* L);
int lStmt_bindInt(lua_State* L);
int lStmt_bindNum(lua_State* L);
int lStmt_bindStr(lua_State* L);
int lStmt_bindBlob(lua_State* L);
int lStmt_colInt(lua_State* L);
int lStmt_colNum(lua_State* L);
int lStmt_colStr(lua_State* L);
int lStmt_colBlob(lua_State* L);
int lStmt_isNull(lua_State* L);

static luaL_Reg lStmt_table[] = {
	{ NULL,NULL }
};
static luaL_Reg lStmt_meta[] = {
	{"indexOf",lStmt_indexOf},
	{ "step", lStmt_step },
	{ "stepAll", lStmt_stepAll },
	{ "reset", lStmt_reset },
	{ "bindInt", lStmt_bindInt},
	{ "bindNum", lStmt_bindNum },
	{ "bindStr", lStmt_bindStr },
	{ "bindBlob", lStmt_bindBlob },
	{ "colInt", lStmt_colInt },
	{ "colNum", lStmt_colNum },
	{ "colStr", lStmt_colStr },
	{ "colBlob", lStmt_colBlob },
	{ "isNull" , lStmt_isNull},
	{ NULL,NULL }
};

int lBlob_alloc(lua_State* L);
int lBlob_free(lua_State* L);
int lBlob_get(lua_State* L);
int lBlob_set(lua_State* L);
int lBlob_clear(lua_State* L);

static luaL_Reg lBlob_table[] = {
	{"alloc", lBlob_alloc},
	{"free" , lBlob_free},
	{NULL,NULL}
};
static luaL_Reg lBlob_meta[] = {
	{"get",lBlob_get},
	{"set",lBlob_set},
	{"clear",lBlob_clear},
	{NULL,NULL}
};
int luaopen_ScriptLibs(lua_State* L);

#endif