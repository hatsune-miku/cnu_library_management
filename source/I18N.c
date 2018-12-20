
/*
	Name:			I18N.c
	Description:	Implementation of I18N.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include "I18N.h"
#include <stdlib.h>

// defination.
int global_language;

bool switchLanguage(int language) {
	int backup = global_language;
	char* name;
	global_language = language;
	if ((name = str("name")) == NULL) {
		// recover change if language does not exist.
		global_language = backup;
		return false;
	}
	setInt(PATH_CONFIG, "language", global_language);
	free(name);
	return true;
}

char* str(const char* key) {
	char file_name[FILE_NAME_SIZE];
	sprintf(file_name, LANG_PATH, global_language, ".ini");
	char* ret = config(file_name, key);
	for (int i = 0; i < LINE_SIZE; i++) {
		if (ret[i] == '\0')
			break;
		if (ret[i] == '\\') {
			// interpret \n and \t.
			ret[i] = ' ';
			switch (ret[i + 1]) {
			case 't':
				ret[i + 1] = '\t';
				break;
			case 'n':
				ret[i + 1] = '\n';
				break;
			default:
				ret[i + 1] = '\t';
				break;
			}
			i += 1;
		}
	}
	return ret;
}
