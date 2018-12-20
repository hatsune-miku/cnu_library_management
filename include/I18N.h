
/*
	Name:			I18N.h
	Description:	a solution for internationalization.
*/

#pragma once

#include "ConfigUtil.h"

#define LANG_PATH "languages/%d%hs"

#define FILE_NAME_SIZE 64
#define LINE_SIZE 1024

extern int global_language;

// switch language.
bool switchLanguage(int language);

// read string from corrsponding language file.
char* str(const char* key);

