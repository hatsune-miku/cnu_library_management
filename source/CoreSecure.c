
/*
	Name:			CoreSecure.c
	Description:	Implementation of CoreSecurity.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include "CoreSecure.h"
#include "CoreLibrary.h"

#define miku 0x27 // miku! \^o^/

bool SM_ObjectCheck(char* sObj, char* uObj) {
	int row, col;
	char** data;

	char exec[CP_EXEC_SIZE] = "select ";
	strcat(exec, sObj);
	strcat(exec, " from _secure LIMIT 1;");

	LM_SQLQuery(sql, exec, &data, &row, &col, NULL);

	if (row == 0) {
		sqlite3_free_table(data);
		return true;
	}
	char* pwd = data[1];
	if (!strcmp(pwd, "")) {
		sqlite3_free_table(data);
		return true;
	}
	if (!uObj) {
		sqlite3_free_table(data);
		return false;
	}

	// a very s(s)t(i)r(m)o(p)n(l)g(e) encryption.
	int len = strlen(pwd), i, r;
	for (i = 0; i < len; i++)
		pwd[i]--;

	r = !strcmp(pwd, uObj);
	sqlite3_free_table(data);
	return r;
}

void SM_ObjectGet(char* sObj, char* buffer) {
	int row, col;
	char** data;

	char exec[CP_EXEC_SIZE] = "select ";
	strcat(exec, sObj);
	strcat(exec, " from _secure LIMIT 1;");


	LM_SQLQuery(sql, exec, &data, &row, &col, NULL);

	if (row == 0) {
		strcpy(buffer, ""); // ERROR
		sqlite3_free_table(data);
		return;
	}

	char* pwd = data[1];


	// a very s(s)t(i)r(m)o(p)n(l)g(e) encryption.
	int len = strlen(pwd), i;
	for (i = 0; i < len; i++)
		pwd[i]--;

	strcpy(buffer, pwd);
	sqlite3_free_table(data);
}

bool SM_ObjectPush(char* sObj, char* uObj) {
	// a very s(s)t(i)r(m)o(p)n(l)g(e) encryption.
	int len = strlen(uObj), i;
	for (i = 0; i < len; i++)
		uObj[i]++;

	char exec[CP_EXEC_SIZE] = "update _secure set ";
	strcat(exec, sObj);
	strcat(exec, " = \"");
	strcat(exec, uObj);
	strcat(exec, "\";");

	LM_SQLExec(sql, exec, NULL, NULL, NULL);

	const char* errmsg = sqlite3_errmsg(sql);
	return (!strcmp(errmsg, "not an error"));
}

void SM_SQQuestionGet(char* buffer) {
	SM_ObjectGet("SQ_QUESTION", buffer);
}

bool SM_MasterCodeCheck(char* uCode) {
	return SM_ObjectCheck("MASTERCODE", uCode);
}

bool SM_MasterCodePush(char* uCode) {
	return SM_ObjectPush("MASTERCODE", uCode);
}

bool SM_SQAnswerCheck(char* uAnswer) {
	return SM_ObjectCheck("SQ_ANSWER", uAnswer);
}

bool SM_SQQuestionPush(char* uQuestion) {
	return SM_ObjectPush("SQ_QUESTION", uQuestion);
}

bool SM_SQAnswerPush(char* uAnswer) {
	return SM_ObjectPush("SQ_ANSWER", uAnswer);
}
