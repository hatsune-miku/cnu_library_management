
/*
	Name:			CoreUI.h
	Description:	Simulate UI in CLI.
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "I18N.h"

#define UI_MENU_HEIGHT 64
#define UI_MENU_WIDTH 512
#define UI_ATTR_FORMAT_SIZE 64

// clear CLI.
// Windows: cls
// Linux: clear
void UI_Clear( void );

// set locale to compatible with unicode charset.
void UI_InitWithUnicodeCS( void );

// read a line from CLI and convert to int with advatoi.
int UI_Input( void );

// get command to clear CLI in current operating system.
const char* clearCmd( void );

// get command to list directory in current operating system.
// Windows: dir
// Linux: ls
const char* lsCmd(void);

// print array in menu format.
void UI_MenuMake(const char* title, int num, ...);

// only works on Debug mode.
void dbgprintf(const char* str);

// add book navigation.
void UI_AddBookNavi( void );

// print book attrs table header.
void UI_PrintBookHeader( FILE* target );

// advanced printf which automatically use str().
void UI_Printf(const char* format, ...);
void UI_Fprintf(FILE* target, const char* format, ...);

// simulate a full-screen MessageBox in CLI.
void UI_MessageBoxWithType(const char* msg, const char* caption, bool tryGUI, unsigned int type);
void UI_MessageBox(const char* msg, bool tryGUI);

// check password.
bool UI_MastercodeCheck(bool flagFullScreen);

// check security question.
bool UI_SQCheck( void );

// wraps UI_MastercodeInputWithCustomMsg().
void UI_MastercodeInputSolution(char* 
	uPassword, bool tryGUI);

// input password in CLI. (uses _getch())
void UI_MastercodeInputWithCustomMsg(
	char* msgTag, 
	char* uPassword);

// another way to print a menu.
void UI_MenuMakeWithArray(
	const char* title,
	int num,
	const char** arr);

// wraps UI_MenuMakeWithArray().
char** UI_MenuMakeWithString(
	const char* title,
	const char* str);

// analyze and print sqlite3 result.
void UI_ListMakeWithSqlite3Array(
	const char* title,
	void(*HeaderProcessor)(FILE*),
	int nrow,
	int ncol,
	bool flagShowDeleted,
	int base,
	char** arr,
	void(*callback)(char*, FILE*),
	FILE* target);

// a switch navigation.
void UI_SwitchMake(
	const char* name, 
	const char* intro, 
	const char* key);

// a setting navigation.
void UI_SettingsMake(
	const char* name, 
	const char* intro, 
	const char* key, 
	bool InputOrSwitch);

// determines raw input or select from given list for each attr.
void UI_AttributedInput(int index, char* name, char* target);

// ask for yes or no.
bool UI_Prompt(const char* msg, bool tryGUI);
