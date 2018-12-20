
/*
	Name:			AlgorithmUtil.c
	Description:	Implementation of AlgorithmUtil.h
*/

// scanf is deprecated but not all compilers support scanf_s etc.
#define _CRT_SECURE_NO_WARNINGS 

#include "AlgorithmUtil.h"
#include <string.h>
#include <stdio.h>
#ifndef _WIN32
#include <iconv.h>
#else
#include <Windows.h>
#endif

// -+-+-+-+-+-+-+-+-+-+-+- global var -+-+-+-+-+-+-+-+-+-+-+-+-+-

// c2w
wchar_t*	global_pw = NULL;

// -+-+-+-+-+-+-+-+-+-+-+- global var -+-+-+-+-+-+-+-+-+-+-+-+-+-

void ascii2Utf8(char* ascii, size_t asciiSize, char* utf8) {
#ifndef _WIN32
    size_t utf8Size = AU_LINE_SIZE;
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (!cd)
        return;
    iconv(cd, (char**) &ascii, &asciiSize, &utf8, &utf8Size);
    iconv_close(cd);
    utf8[asciiSize] = '\0';
#else
	UNREFERENCED_PARAMETER(asciiSize);
	char* pg = U2G(ascii);
	strcpy(utf8, pg);
	free(pg);
#endif
}

// need free!
char* G2U(const char* gb2312) {
#ifdef _WIN32
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = (char*)malloc((len + 1) * sizeof(char));
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr)
		free(wstr);
	return str;
#else
	return gb2312;
#endif
}

// need free!
char* U2G(const char* utf8) {
#ifdef _WIN32
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = malloc((len + 1) * sizeof(char));
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr)
		free(wstr);
	return str;
#else
	return utf8;
#endif
}

void advitoa(int i, char* w) {
	int bi = i, len = 0;
	int dg[10];
	if (bi == 0) {
		*(w++) = '0';
		*w = '\0';
		return;
	}
	while (bi) {
		dg[len++] = bi % 10;
		bi /= 10;
	}
	while (len--)
		*(w++) = (char)(dg[len] + 48);
	*w = '\0';
}

wchar_t* c2w(char* c) {
	if (!c)
		return NULL;

	if (!global_pw)
		global_pw = malloc(AU_LINE_SIZE * sizeof(char));

	size_t wsize = wcslen((wchar_t*)c);

	mbstowcs(global_pw, c, wsize);
	return global_pw;
}

int advatoi(const char* str) {
	int ret = 0, base = 1, len = strlen(str), i;
	int flagMinus = 1;
	for (i = len - 1; i >= 0; i--) {
		if (i == 0 && str[i] == '-') {
			flagMinus = -1;
			break;
		}
		if ('0' <= str[i] && str[i] <= '9') {
			ret += (int)(str[i] - 48) * base;
			base *= 10;
		}
	}
	return flagMinus * ret;
}

void readline(char* str, int limit) {
	char c;
	while ((c = (char)getchar()) != '\r' && c != '\n' && c != EOF)
		if (--limit)
			*(str++) = c;
		else
			scanf("%*s"); // aw
	*str = '\0';
}


void split(const char* of, const char with, char** ret, int* n) {
	*n = 0;
	int j = 0;
	while (*of != '\0') {
		if (*of == with) {
			ret[*n][j] = '\0';
			(*n)++;
			j = 0;
		}
		else
			ret[*n][j++] = *of;
		of++;
	}
	ret[*n][j] = '\0';
	(*n)++;
}

int wstrlen(const char* str) {
#ifndef _WIN32
	return strlen(str);
#endif
	int size = 0, i, len = strlen(str);
	bool flagWchar = false;
	for (i = 0; i < len; i++) {
		if (str[i] > CHAR_MAX) {
			size += sizeof(char);
			flagWchar = true;
		}
		else
			size += 1;
	}
	if (!flagWchar)
		size--;
	return size;
}

int uniArrIndexOf(void** of, void* with, bool(*Compare)(void* a, void* b), int limit) {
	int ret;
	for (ret = 0; ret < limit; ret++) {
		if (Compare(of[ret], with))
			return ret;
	}
	return -1;
}

bool __sequals(char* a, char* b) {
	return !strcmp(a, b);
}

bool __wsequals(char* a, char* b) {
	return !strcmp(a, b);
}

int arrIndexOf(char** of, char* with, int limit) {
	return uniArrIndexOf(of, with, __sequals, limit);
}

int arrIndexOfInt(int* of, int with, int limit) {
	int ret;
	for (ret = 0; ret < limit; ret++)
		if (of[ret] == with)
			return ret;
	return -1;
}

void trim(char* r, int limit) {
	while (limit--) {
		if (*r == '\r' || *r == '\n') {
			*r = '\0';
		}
		r--;
	}
}
