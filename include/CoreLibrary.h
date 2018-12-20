
/*
	Name:			CoreLibrary.h
	Description:	Core library management. 
*/

#pragma once

#include "sqlite3-secure/sqlite3.h"
#include "NewStd.h"
#include "CoreProgram.h"

#define LM_BUFFER_SIZE 1024
#define PATH_CONDITION_FILE "conds\\"

// order of attrs.
extern int* global_order;

// shared sql object.
extern sqlite3* sql;

// more convenient than a lot of malloc.
char** LM_DynamicArray2Make(int width, int height);
void LM_FreeDynamicArray(void** arr, int n);

// initialize function.
bool LM_Init( void );

// reset database and rebuild attr structure.
bool LM_Reset(bool sure);

// wraps sqlite3_exec.
int LM_SQLExec(
	sqlite3* obj, 
	char* exec, 
	int(*callback)(void*, int, char**, char**), 
	void* preserved, 
	char** errmsg);

// wraps sqlite3_get_table.
int LM_SQLQuery(
	sqlite3* obj,
	char* exec,
	char*** pszResult,
	int* nrow,
	int* ncol,
	char** pszErrmsg);


// fetch all books from sqlite.
void LM_QueryBookAll(
	int* nrow,
	int* ncol,
	char*** result,
	int limit,
	int offset);

// fetch books with custom sqlite exec.
void LM_QueryBookWithSqlExec(
	char* exec,
	int* nrow,
	int* ncol,
	char*** result);

// mark if a book is deleted.
void LM_ChangeBookDeleteStatWithConds(
	int n,
	const char* stat,
	CP_Condition* conds,
	bool AND);

// convert CP_Condition[] to sqlite exec.
void LM_ConditionExecMakeOpt(
	const char* header,
	int n,
	CP_Condition* conds,
	char* result,
	const char* customParam,
	int limit,
	int offset,
	bool AND);

// (generate exec only) - fetch books with CP_Conditions.
void LM_SelectConditionExecMake(
	int n,
	CP_Condition* conds,
	char* result,
	int limit,
	int offset,
	bool AND);

// (generate exec only) - mark some books as deleted.
void LM_DeleteConditionExecMake(
	int n,
	CP_Condition* conds,
	char* result,
	int limit,
	int offset,
	bool AND);

// (action) - delete books.
void LM_DeleteBooksWithConds(
	int n, 
	CP_Condition* conds, 
	bool AND);

// (action) - delete books permanently. 
void LM_RemoveBooksWithConds(
	int n,
	CP_Condition* conds,
	bool AND);

// (action) - recover deleted books.
void LM_RescueBooksWithConds(
	int n, 
	CP_Condition* conds, 
	bool AND);

// (action) - modify books.
void LM_UpdateExecMakeWithConds(
	int nConds,
	CP_Condition* conds,
	int n,
	char** modifiedItems,
	char** newValues,
	char* exec);

// fetch a specified book.
void LM_QueryBookWithBookNumber(
	char* bookNumber, 
	int* nrow, 
	int* ncol, 
	char*** result);

// (action) - add books from files in prepared formats.
bool LM_AddBookWithFile(
	char* path,
	int n, 
	int* _order,
	int* nsuc,
	int* nfail);
void LM_ConditionFileLoad(
	char* condName,
	CP_Condition* conds,
	int* condition_cnt);


// (action) - add books with given args.
bool LM_AddBookWithOrder(
	char** args, 
	int n, 
	int* order,
	bool flagUseOrder);

bool UI_BookModifyWithGUI(char* bookNumber);
bool LM_Add(char** args, int n, bool flagUseOrder);

// generate a CP_Condition file.
void LM_ConditionFileGen(int n, char* condName,
	CP_Condition* conds);

// get a specified value of a book.
void LM_ValueGetWithBookNumber(char* bookNumber, char* key, char** pvalue);
void LM_ValueGetWithBookNumberFree(char** pvalue);

// translates CP_Condition.cond
void LM_CondRulesCat(char* result, char cond);

void LM_GetOrder(int* _order, int n);
void LM_GetOrderWithCondName(const char* condName, int* _order, int n);


// int LM_Callback(void* NotUsed, int argc, char** argv, char** argn);
// deprecated due to a priv violation (directly LM->UI).
