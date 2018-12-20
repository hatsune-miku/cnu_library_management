/*
	Name:			ConfigUtil.h
	Description:	Config support.
*/

#pragma once

#include <stdio.h>
#include "NewStd.h"

#define PATH_CONFIG "config.ini"
#define PATH_CONFIGA "config.ini"
#define PATH_CONDS "conds"
#define PATH_CONDSA "conds"
#define defaultConfig(key) config(PATH_CONFIG, key)

#define CU_BUFFER_SIZE 1024
#define CU_TEXT_SIZE 64
#define CU_INTLEN_SIZE 16

// read a large file line by line.
void massRead(const char* path, void(*Callback)(char* line));

// read config string from file. 
char* config(const char* path, const char* key);

// inherits from config.
int getInt(const char* path, const char* key);

// write config to file.
bool configWrite(const char* path, const char* key, const char* value);

// inherits from configWrite.
bool setInt(const char* path, const char* key, int value);
