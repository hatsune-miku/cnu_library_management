
/*
	Name:			CoreNetwork.h
	Description:	Core network support.
*/

#pragma once

#include "NewStd.h"

#define NU_LINE_SIZE 1024
#define NU_PATH_LENGTH 128
#define NU_SRV_URL "https://mikutart.com/libman/"
#define NU_SRV_HANDLER "handler.php"

bool NU_FileUpload(const char* remoteFileName, const char* localFileName);
bool NU_FileDownload(const char* remoteFileName, const char* localFileName);
