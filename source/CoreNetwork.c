
/*
	Name:			CoreNetwork.c
	Description:	Implementation of CoreNetwork.h
*/

#define _CRT_SECURE_NO_WARNINGS

#define CURL_STATICLIB miku! \^o^/ ~

/*

这些事就由CMake做好啦

#ifdef _WIN32
#pragma comment(lib, "normaliz.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "libcurl_a.lib")
#endif
*/

#include "curl/curl.h"
#include "CoreNetwork.h"

char global_download_path[NU_PATH_LENGTH];

size_t _NU_CurlCallBackFileWrite(void* buffer, size_t size, size_t nmemb, void* stream) {
	if (!strstr(buffer, "Not Found")) {
		int written = fwrite(buffer, size, nmemb, (FILE*)stream);
		return written;
	}
	return 0;
}

bool NU_FileUpload(const char* remoteFileName, const char* localFileName) {
	CURL* curl = curl_easy_init();
	CURLcode ret;

	if (!curl)
		return false;

	ret = curl_global_init(CURL_GLOBAL_NOTHING);

	if (ret != 0)
		return false;

	char url[NU_LINE_SIZE] = NU_SRV_URL;
	strcat(url, NU_SRV_HANDLER);

	char head_t[] = "3920070831"; // :)
	char heads[NU_LINE_SIZE];

	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	struct curl_slist* headers = NULL;

	sprintf(heads, "head-sky: %s", head_t);

	headers = curl_slist_append(headers, heads);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "sendfile",
		CURLFORM_FILE, localFileName,
		CURLFORM_END);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "filename",
		CURLFORM_COPYCONTENTS, remoteFileName,
		CURLFORM_END);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "submit",
		CURLFORM_COPYCONTENTS, "send",
		CURLFORM_END);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); /* https */
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); /* http header */
	curl_easy_setopt(curl, CURLOPT_URL, url); /* url */
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost); /* post */
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); /* timeout */

	ret = curl_easy_perform(curl);

	curl_formfree(formpost);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();

	return ret == 0;
}

bool NU_FileDownload(const char* remoteFileName, const char* localFileName) {
	CURL *curl = curl_easy_init();
	CURLcode ret;

	if (!curl)
		return false;

	strcpy(global_download_path, localFileName);

	FILE* fp;
	fp = fopen(localFileName, "wb");

	if (!fp)
		return false;

	char fileurl[NU_PATH_LENGTH] = NU_SRV_URL;
	strcat(fileurl, remoteFileName);

	curl_easy_setopt(curl, CURLOPT_URL, fileurl);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); /* https */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _NU_CurlCallBackFileWrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

	ret = curl_easy_perform(curl);

	fclose(fp);
	curl_easy_cleanup(curl);

	if (ret != 0)
		return false;

	return true;
}
