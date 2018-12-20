/*
	Name:			BookAttr.h
	Description:	Implementaion of BookAttr.h
*/

#define _CRT_SECURE_NO_WARNINGS

// #include <sqlite3.h>
#include <stdio.h>
#include <string.h>

#include "ConfigUtil.h"
#include "BookAttr.h"
#include "AlgorithmUtil.h"

int BA_GetNattr(void) {
	return getInt(PATH_BOOK_ATTR, "nattr");
}

int BA_GetCreateNattr(void) {
	return getInt(PATH_BOOK_ATTR, "create_nattr");
}

void BA_GetAttr(int index, char* name, char* type) {
	if (index < 0)
		return;

	char num[4];
	advitoa(index, num);

	char* name_type = config(PATH_BOOK_ATTR, num);

	int dot_pos = 0;
	while (name_type[dot_pos] != '\0') {
		if (name_type[dot_pos] == ',') {
			name_type[dot_pos] = '\0'; // split from ,.
			break;
		}
		dot_pos++;
	}
	if (name)
		strcpy(name, name_type);
	if (type)
		strcpy(type, name_type + dot_pos + 1);

	free(name_type);
}

void BA_NameForType(char* name, char* type) {
	int n = BA_GetNattr(), i;
	char tname[BA_NAME_SIZE], ttype[BA_TYPE_SIZE];
	for (i = 0; i < n; i++) {
		// violate search.
		BA_GetAttr(i, tname, ttype);
		if (!strcmp(tname, name)) {
			strcpy(type, ttype);
			return;
		}
	}
	strcpy(type, ""); // ERROR
}

char* BA_NameForLabel(char* name, char label) {
	int n = BA_GetNattr();
	char tname[BA_NAME_SIZE];
	for (int i = 0; i < n; i++) {
		BA_GetAttr(i, tname, NULL);
		if (!strcmp(tname, name))
			return BA_GetLabel(i, label);
	}
	return NULL;
}

// TODO: memory problem.
char* BA_GetLabel(int index, char label) {
	char raw[4] = "%d";
	char format[BA_FORMAT_SIZE];
	raw[2] = label;
	sprintf(format, raw, index);
	char* ret = config(PATH_BOOK_ATTR, format);
	return ret;
}
