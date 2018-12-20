
/*
	Name:			AlgorithmUtil.h
	Description:	Algorithm support. 
*/

#pragma once

#include "NewStd.h"
#include <stdlib.h>

#define AU_LINE_SIZE 1024

/*
	Umm... This AlgorithmUtil doesn't have any awesome algorithms.
	coz this program bases on sqlite3 who did everything well...
*/

// converts char[] to wchar_t[].
wchar_t* c2w(char* c);

// convert gb2312 between utf-8.
char* G2U(const char* gb2312);
char* U2G(const char* utf8);

// solve SQLite encoding issue in Linux.
void ascii2Utf8(char* ascii, size_t asciiSize, char* utf8);

// converts int to char[]
void advitoa(int i, char* w);

// has a higher accuracy than strlen :)
int wstrlen(const char* str);

// "1aaa2bb3ccccc4dd5eee" -> 12345
int advatoi(const char* str);

// they find out a element in an array.
int uniArrIndexOf(void** of, void* with, bool(*Compare)(void* a, void* b), int limit);
int arrIndexOf(char** of, char* with, int limit);
int arrIndexOfInt(int* of, int with, int limit);

// I wrote this after scanf("[\^n]", ) crashed for 1,000,000,000,000,000,000,000 times.
void readline(char* str, int limit);

// drops \r and \n at the end of string.
void trim(char* r, int limit);

// so why doesn't string.h provide split?!
void split(const char* of, const char with, char** ret, int* n);
