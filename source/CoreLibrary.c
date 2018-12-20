
/*
	Name:			CoreLibrary.c
	Description:	Implementation of CoreLibrary.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>

#include "CoreLibrary.h"
#include "AlgorithmUtil.h"
#include "CoreUI.h"
#include "BookAttr.h"
#include "ConfigUtil.h"

#define SQL_COMMIT_BOUNDARY 1000

sqlite3* sql;
int* global_order;
// defination part

char** LM_DynamicArray2Make(int width, int height) {
	char** tmp = (char**)malloc(height * sizeof(char*));
	for (int i = 0; i < height; i++)
		tmp[i] = (char*)malloc(width * sizeof(char));
	return tmp;
}

void LM_FreeDynamicArray(void** arr, int n) {
	for (int i = 0; i < n; i++)
		free(arr[i]);
	free(arr);
}


void LM_GetOrderWithCondName(const char* condName, int* _order, int n) {
	char* raw = config(PATH_BOOK_ATTR, condName);
	int i, sum = 0, base = 1,
		len = strlen(raw), nattr = n;

	for (i = len - 1; i >= 0; i--) {
		if (raw[i] == ',') {
			_order[--nattr] = sum;
			sum = 0;
			base = 1;
		}
		else {
			sum += base * (int)(raw[i] - 48);
			base *= 10;
		}
	}
	_order[--nattr] = sum;
}

void LM_GetOrder(int* _order, int n) {
	LM_GetOrderWithCondName("order", _order, n);
}

// Deprecated
/*
int LM_Callback(void* NotUsed, int argc, char **argv, char **argn) {
	int i;
	char item[4], name[64], type[16];
	for (i = 1; i < argc; i++) {
		sprintf(item, 4, "%di", order[i - 1]);
		int width = getInt(PATH_BOOK_ATTR, item);
		BA_GetAttr(order[i - 1], name, type);

		char format[8];
		sprintf(format, 8, "%%-%ds", width);
		printf(format, (argv[i]));
	}
	putchar('\n');
	return 0;
}
*/

bool LM_AddBookWithFile(char* path, int n, int* order, int* nsuc, int* nfail) {

	FILE* hFile = fopen(path, "r");
        
	if (!hFile)
		return false;

	char wline[CP_EXEC_SIZE];
	char** data = LM_DynamicArray2Make(CP_EXEC_SIZE, n);

	int i = 0, nline = 0;
	int nattr = BA_GetNattr();
	char** args = LM_DynamicArray2Make(CP_COND_VALUE_SIZE, nattr);

	LM_SQLExec(sql, "begin;", NULL, NULL, NULL);

	while (fgets(wline, CP_EXEC_SIZE, hFile) != NULL) {
		if (wline[0] == ';' || wline[1] == ';')
			continue;

		trim(wline + CP_EXEC_SIZE - 1, CP_EXEC_SIZE);

		int an = 0;
		split(wline, ' ', data, &an);

		nline++;
		if (nline >= SQL_COMMIT_BOUNDARY) {
			// commit per 1000 data
			nline = 0;
			LM_SQLExec(sql, "commit;", NULL, NULL, NULL);
			LM_SQLExec(sql, "begin;", NULL, NULL, NULL);
		}
		
		char name[CP_COND_NAME_SIZE], type[CP_COND_NAME_SIZE];

		for (i = 0; i < nattr; i++) {
			BA_GetAttr(global_order[i], name, type);
			int index = arrIndexOfInt(order, global_order[i], n);

			// perform default
			if (index == -1) {
				if (!strcmp(type, "TEXT"))
					strcpy(args[i], "");
				else
					strcpy(args[i], "0");
			}
			else {
				strcpy(args[i], (data[index]));
			}
		}

		printf("\n  > %s", args[0]);
		// <debug>
		

		if (LM_AddBookWithOrder(args, nattr, global_order, false)) {
			(*nsuc)++;
			printf(" - ok");
		}
		else {
			printf(" - fail");
			(*nfail)++;
		}
	}
	LM_SQLExec(sql, "commit;", NULL, NULL, NULL);

	fclose(hFile);
	LM_FreeDynamicArray(data, n);
	LM_FreeDynamicArray(args, nattr);
	return true;
}

bool LM_Add(char** args, int n, bool flagUseOrder) {
	return LM_AddBookWithOrder(args, n, global_order, flagUseOrder);
}

bool LM_AddBookWithOrder(char** args, int n, int* order, bool flagUseOrder) {
	char exec[CP_EXEC_SIZE] = "insert into book VALUES(NULL, ";
	int i, nattr = BA_GetNattr();
	for (i = 0; i < n; i++) {
		if (i != 0)
			strcat(exec, ", ");

		char name[CP_COND_VALUE_SIZE], type[CP_COND_VALUE_SIZE];
		BA_GetAttr(order[i], name, type);
		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			strcat(exec, "\"");
		if (flagUseOrder)
			strcat(exec, args[order[i]]);
		else
			strcat(exec, args[i]);
		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			strcat(exec, "\"");
	}

	for (i = 0; i < nattr - n; i++)
		strcat(exec, ", 0");
	strcat(exec, ", 0);");
	return LM_SQLExec(sql, exec, NULL, NULL, NULL) == SQLITE_OK;	
}

void LM_CustomSqliteExec(char* command, int(*Callback)(void*, int, char**, char**)) {
	sqlite3_exec(sql, command, Callback, NULL, NULL);
}

void LM_ConditionFileLoad(char* condName, CP_Condition* conds, int* condition_cnt) {
	char fileName[CP_COND_SIZE] = PATH_CONDITION_FILE;
	strcat(fileName, condName);
	strcat(fileName, ".cond");
	*condition_cnt = getInt(fileName, "cnt");
	int i;
	for (i = 0; i < *condition_cnt; i++) {
		char format[CP_TFORMAT_SIZE];
		sprintf(format, "%d", i + 1);
		char* raw = (config(fileName, format));
		sscanf(raw, "%[^,]%*c%c%*c%s", conds[i].name,
			&conds[i].cond, conds[i].value);
	}
}

void LM_ConditionFileGen(int n, char* condName, CP_Condition* conds) {
	char buffer[LM_BUFFER_SIZE] =
		"; Auto-generated condition file\n\n[cond]\ncnt=";
	char fileName[CP_COND_SIZE] = PATH_CONDITION_FILE;
	strcat(fileName, condName);
	strcat(fileName, ".cond");
        
	FILE* hFile = fopen(fileName, "wb");
	if (!hFile)
		return;
	fprintf(hFile, buffer);
	fprintf(hFile, "%d\n", n);
	int i;
	for (i = 0; i < n; i++) {
		char wname[CP_COND_NAME_SIZE];
		strcpy(wname, (conds[i].name));
		char wvalue[CP_COND_VALUE_SIZE];
		strcpy(wvalue, (conds[i].value));
		fprintf(hFile, "%d=%s,%c,%s\n", i + 1, wname, conds[i].cond, wvalue);
	}
	fclose(hFile);
}

int LM_SQLQuery(sqlite3* obj, char* exec, char*** pszResult, int* nrow, int* ncol, char** pszErrmsg) {
	char* pexec = G2U(exec);
	int ret = sqlite3_get_table(obj, pexec, pszResult, nrow, ncol, pszErrmsg);
	free(pexec);
	return ret;
}

int LM_SQLExec(sqlite3* obj, char* exec, int (*callback)(void*, int, char**, char**), void* preserved, char** errmsg) {
	char* pexec = G2U(exec);
	int ret = sqlite3_exec(obj, pexec, callback, preserved, errmsg);
	free(pexec);
	return ret;
}

bool LM_Init(void) {
	int nattr = BA_GetCreateNattr();
	global_order = (int*)malloc(nattr * sizeof(int));
	LM_GetOrder(global_order, nattr);

	bool open = !sqlite3_open(PATH_DATABASE, &sql);
	if (!open)
		return false;

	sqlite3_key(sql, "miku", 4);
	LM_SQLExec(sql, "PRAGMA case_sensitive_like = 0;", NULL, NULL, NULL);
	return true;
}

bool LM_Reset(bool sure) {
	if (!sure)
		return false;

/*
	sqlite3_close(sql);
	if (sqlite3_open(PATH_DATABASE, &sql))
		return false;

	sqlite3_key(sql, "miku", 4);
*/
	// LM_SQLExec(sql, ".backup main book_backup.db", NULL, NULL, NULL)
	// dbgprintf(sqlite3_errmsg(sql));

	LM_SQLExec(sql, "drop table book;", NULL, NULL, NULL);
	LM_SQLExec(sql, "drop table _secure;", NULL, NULL, NULL);
	// delete all tables.

	LM_SQLExec(sql, "create table _secure(MASTERCODE TEXT, SQ_QUESTION TEXT, SQ_ANSWER TEXT);", NULL, NULL, NULL);
	LM_SQLExec(sql, "insert into _secure VALUES(\"\",\"\",\"\")", NULL, NULL, NULL);

	char exec[CP_EXEC_SIZE] = "create table book(ID INTEGER PRIMARY KEY autoincrement, ";
	// rebuild table with new attrs.

	char name[CP_COND_SIZE], type[CP_COND_SIZE];
	int i, cnattr = BA_GetCreateNattr();
	for (i = 0; i < cnattr; i++) {
		if (i != 0)
			strcat(exec, ", ");
		BA_GetAttr(global_order[i], name, type);
		strcat(exec, (name));
		strcat(exec, " ");
		strcat(exec, (type));
	}

	strcat(exec, ");");
	LM_SQLExec(sql, exec, NULL, NULL, NULL);
	dbgprintf(sqlite3_errmsg(sql));

	return true;
}

void LM_ChangeBookDeleteStatWithConds(int n, const char* stat, CP_Condition* conds, bool AND) {
	(AND);
	char exec[CP_EXEC_SIZE];
	char** modifiedItems = LM_DynamicArray2Make(CP_COND_NAME_SIZE, 1);
	strcpy(modifiedItems[0], "PRESERVED_DELETED");
	// Uses a tag marking deleted without really delete it.

	char** newValues = LM_DynamicArray2Make(CP_COND_NAME_SIZE, 1);
	strcpy(newValues[0], stat);

	LM_UpdateExecMakeWithConds(n, conds, 1, modifiedItems, newValues, exec);
	LM_SQLExec(sql, exec, NULL, NULL, NULL);

	LM_FreeDynamicArray((void**)modifiedItems, 1);
	LM_FreeDynamicArray((void**)newValues, 1);
}

void LM_DeleteBooksWithConds(int n, CP_Condition* conds, bool AND) {
	LM_ChangeBookDeleteStatWithConds(n, "1", conds, AND);
}

void LM_RescueBooksWithConds(int n, CP_Condition* conds, bool AND) {
	LM_ChangeBookDeleteStatWithConds(n, "0", conds, AND);
}

void LM_RemoveBooksWithConds(int n, CP_Condition* conds, bool AND) {
	char exec[CP_EXEC_SIZE];
	LM_DeleteConditionExecMake(n, conds, exec, -1, -1, AND);
	LM_SQLExec(sql, exec, NULL, NULL, NULL);
}

// need free!
void LM_QueryBookWithSqlExec(
	char* exec,
	int* nrow,
	int* ncol,
	char*** result) {
	LM_SQLQuery(sql, exec, result, nrow, ncol, NULL);
}

void LM_UpdateExecMakeWithConds(int nConds, CP_Condition* conds, int n, char** modifiedItems, char** newValues, char* exec) {
	strcpy(exec, "update book set ");

	int i;
	char type[BA_TYPE_SIZE];

	for (i = 0; i < n; i++) {

		if (i != 0)
			strcat(exec, ", ");

		char* name = modifiedItems[i];
		char* value = newValues[i];
		bool flagText = false;

		BA_NameForType((name), type);
		strcat(exec, name);
		strcat(exec, " = ");
		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			flagText = true;

		if (flagText)
			strcat(exec, "\"");
		strcat(exec, value);
		if (flagText)
			strcat(exec, "\"");
	}
	char end[CP_EXEC_SIZE / 2];
	if (nConds == 0) {
		sprintf(end, ";");
	}
	else {
		sprintf(end, " where ");
		for (i = 0; i < nConds; i++) {

			if (i != 0)
				strcat(exec, ", ");

			bool flagText = false;

			BA_NameForType(conds[i].name, type);
			strcat(end, conds[i].name);
			LM_CondRulesCat(end, conds[i].cond);

			if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
				flagText = true;

			if (flagText)
				strcat(end, "\"");
			strcat(end, conds[i].value);
			if (flagText)
				strcat(end, "\"");
		}
	}
	strcat(exec, end);
	strcat(exec, ";");
}

/*
void LM_UpdateExecMake(char* id, int n, char** modifiedItems, char** newValues, char* exec) {
	strcpy(exec, "update book set ");
	int i;
	char type[16];
	for (i = 0; i < n; i++) {
		if (i != 0)
			strcat(exec, ", ");
		char* name = modifiedItems[i];
		char* value = newValues[i];
		// bool
		BA_NameForType((name), type);
		strcat(exec, name);
		strcat(exec, " = ");
		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			strcat(exec, "\"");
		strcat(exec, value);
		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			strcat(exec, "\"");
	}
	char end[16];
	sprintf(end, " where id = %s;", id);
	strcat(exec, end);
}
*/
// deprecrated.

void LM_CondRulesCat(char* result, char cond) {
	switch (cond) {
	case '=': // =
		strcat(result, " = ");
		break;
	case '>': // >
		strcat(result, " > ");
		break;
	case '<': // <
		strcat(result, " < ");
		break;
	case 'L': // Looks like
		strcat(result, " LIKE ");
		break;
	case 'N': // !=
		strcat(result, " != ");
		break;
	}
}

/* set limit = -1 to delete */
void LM_ConditionExecMakeOpt(const char* header, int n, CP_Condition* conds, char* result, const char* customParam, int limit, int offset, bool AND) {

	strcpy(result, header);
	strcat(result, "from book ");
	int i;
	char type[BA_TYPE_SIZE];

	if (n > 0)
		strcat(result, "where ");

	for (i = 0; i < n; i++) {
		if (i != 0) {
			if (!strcmp(conds[i].name, "PRESERVED_DELETED"))
				strcat(result, " AND ");
			else
				strcat(result, AND ? " AND " : " OR ");
		}
		strcat(result, conds[i].name);
		LM_CondRulesCat(result, conds[i].cond);
		BA_NameForType((conds[i].name), type);

		bool flagText = false, flagLike = false;

		if (!strcmp(type, "TEXT") || !strcmp(type, "TEXT UNIQUE"))
			flagText = true;

		if (conds[i].cond == 'L')
			flagLike = true;

		if (flagLike)
			strcat(result, "\"%\" || ");
		if (flagText)
			strcat(result, "\"");
		strcat(result, conds[i].value);
		if (flagText)
			strcat(result, "\"");
		if (flagLike)
			strcat(result, " || \"%\"");
	}
	char limit_offset[CP_COND_VALUE_SIZE];
	if (customParam)
		strcat(result, customParam);
	if (limit != -1)
		sprintf(limit_offset, " LIMIT %d OFFSET %d;", limit, offset);
	else
		strcpy(limit_offset, ";");
	strcat(result, limit_offset);
}

void LM_QueryBookWithBookNumber(char* bookNumber, int* nrow, int* ncol, char*** result) {
	CP_Condition cond[1];
	strcpy(cond[0].name, "STR_BOOK_NUMBER");
	strcpy(cond[0].value, bookNumber);
	cond[0].cond = '=';

	char exec[CP_EXEC_SIZE];
	LM_ConditionExecMakeOpt("select * ", 1, cond, exec, "", -1, 0, true);
	LM_QueryBookWithSqlExec(exec, nrow, ncol, result);
}

void LM_SelectConditionExecMake(int n, CP_Condition* conds, char* result, int limit, int offset, bool AND) {
	LM_ConditionExecMakeOpt("select * ", n, conds, result, NULL, limit, offset, AND);
}

void LM_DeleteConditionExecMake(int n, CP_Condition* conds, char* result, int limit, int offset, bool AND) {
	LM_ConditionExecMakeOpt("delete ", n, conds, result, NULL, limit, offset, AND);
}

void LM_ValueGetWithBookNumber(char* bookNumber, char* key, char** pvalue) {
	char exec[LM_BUFFER_SIZE];
	char** data;
	int nrow, ncol;
	sprintf(exec, "select %s from book where STR_BOOK_NUMBER = \"%s\";", key, bookNumber);
	LM_SQLQuery(sql, exec, &data, &nrow, &ncol, NULL);
	if (ncol == 0)
		*pvalue = NULL;
	else {
		*pvalue = (char*)malloc(CP_COND_VALUE_SIZE * sizeof(char));
		strcpy(*pvalue, data[ncol]);
	}
	sqlite3_free_table(data);
}

void LM_ValueGetWithBookNumberFree(char** pvalue) {
	if (*pvalue)
		free(*pvalue);
}

// need free!
void LM_QueryBookAll(int* nrow, int* ncol, char*** result, int limit, int offset) {
	char exec[128];
	sprintf(exec, "select * from book LIMIT %d OFFSET %d;", limit, offset);
	LM_SQLQuery(sql, exec, result, nrow, ncol, NULL);
}

