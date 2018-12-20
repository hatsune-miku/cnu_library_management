/*
	Name:			BookAttr.h
	Description:	Manage attributes of a book.
*/

#pragma once

#include "NewStd.h"
#define PATH_BOOK_ATTR "bookattr.ini"
#define PATH_BOOK_ATTRA "bookattr.ini"
#define PATH_DATABASE "book.db"
#define BA_NAME_SIZE 64
#define BA_FORMAT_SIZE 64
#define BA_TYPE_SIZE 16

// reads the number of attributes without preserved attrs from file.
int BA_GetNattr( void );

// reads the number of all attributes from file.
int BA_GetCreateNattr( void );

// reads an attr's name and type from file.
void BA_GetAttr(int index, char* name, char* type);

// reads an attr's extra data from file.
char* BA_GetLabel(int index, char label);

// find out an attr name's corrsponding type from file.
void BA_NameForType(char* name, char* type);

char* BA_NameForLabel(char* name, char label);
