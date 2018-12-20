/*
	Name:			ConfigUtil.c
	Description:	Implementation of ConfigUtil.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include "ConfigUtil.h"
#include <string.h>
#include <stdlib.h>
#include "AlgorithmUtil.h"

char* line = NULL;
char* ret = NULL;

int getInt(const char* path, const char* key) {
	char* nattr = config(path, key);
	int rint = 0, len = strlen(nattr), i, base = 1;
	for (i = len - 1; i >= 0; i--) {
		if ('0' <= nattr[i] && nattr[i] <= '9') {
			rint += base * ((int)(nattr[i]) - 48);
			base *= 10;
		}
	}
	free(nattr);
	return rint;
}

bool setInt(const char* path, const char* key, int value) {
	char wvalue[CU_INTLEN_SIZE];
	advitoa(value, wvalue);
	return configWrite(path, key, wvalue);
}

void massRead(const char* path, void(*Callback)(char* line)) {
	FILE* hFile = fopen(path, "r");

	if (!hFile)
		return;

	char rline[CU_BUFFER_SIZE];
	while (fgets(rline, CU_BUFFER_SIZE, hFile) != NULL)
		Callback(rline);

	fclose(hFile);
}

char* config(const char* path, const char* key) {
	const char* wpath = path;
	FILE* hFile = fopen((wpath), "r");

	line = (char*)malloc(CU_BUFFER_SIZE * sizeof(char));

	if (!hFile) {
		strcpy(line, ""); // ERROR
		return line;
	}

 	int keylen = strlen(key);
	char _key[CU_TEXT_SIZE];
	char* _value = (char*)malloc(CU_BUFFER_SIZE * sizeof(char));
	
	while (fgets(line, CU_BUFFER_SIZE * sizeof(char), hFile) != NULL) {

		if (line[0] == ';' || line[0] == '[')
			continue;

		line[keylen] = '\0';
		line[CU_BUFFER_SIZE - 1] = '\0';

		strcpy(_key, line);
		strcpy(_value, line + keylen + 1);

		if (!strcmp(key, _key)) {
			// match
			fclose(hFile);
			trim(_value + CU_BUFFER_SIZE - 1, CU_BUFFER_SIZE);
			// ret = (char*)malloc(CU_BUFFER_SIZE * sizeof(char));
			// memset(ret, '\0', CU_BUFFER_SIZE * sizeof(char));
			// strcpy(ret, _value);
			free(line);
			return _value;
		}
		memset(line, '\0', CU_BUFFER_SIZE * sizeof(char));
	}
	free(_value);
	strcpy(line, ""); // ERROR
	fclose(hFile);
	return line;
}

bool configWrite(const char* path, const char* key, const char* value) {
	const char* wpath = path;
	FILE* hFile = fopen(wpath, "r+");

	if (!hFile)
		return false;

	char unidata[CU_BUFFER_SIZE][CU_TEXT_SIZE];

	int keylen = strlen(key), i = 0;
	int valuelen = strlen(value);

	while (fgets(unidata[i], CU_TEXT_SIZE, hFile) != NULL) {
		char b = unidata[i][keylen];
		unidata[i][keylen] = '\0';

		if (!strcmp(key, unidata[i])) {
			// match
			strcpy(unidata[i] + keylen + 1, value);
			strcat(unidata[i] + keylen + 1 + valuelen, "\r\n");
		}
		unidata[i][keylen] = b;
		i++;
		// totally clear and re-write.
	}
	fclose(hFile);

	hFile = fopen(wpath, "w"); // write mode re-open

	if (!hFile)
		return false;

	for (int j = 0; j < i; j++)
		fprintf(hFile, "%s", unidata[j]);

	fclose(hFile);
	return true;
}

