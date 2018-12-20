
/*
	Name:			Main.c
	Description:	where main() locates.
*/

#include <stdio.h>

#include "CoreUI.h"
#include "CoreProgram.h"
#include "CoreLibrary.h"


#ifdef _WIN32
// more pretty GUI.
#pragma comment(linker, "/manifestdependency:\"type='win32' \
						name='Microsoft.Windows.Common-Controls' \
						version='6.0.0.0' \
						processorArchitecture='x86' \
						publicKeyToken='6595b64144ccf1df' \
						language='*'\"")
#endif

int main(void) {
	// unicode charset
	UI_InitWithUnicodeCS();

	// load default language
	int languageId = getInt(PATH_CONFIG, "language");
	switchLanguage(languageId);

	// init sqlite3
	bool flagInitSuc = LM_Init();
	if (!flagInitSuc)
		if (UI_Prompt("STR_ERROR_INIT", true))
			LM_Reset(true); // data lost
		else
			return EXIT_FAILURE;

#ifdef _WIN32
	// bigger size in Windows.
	system("mode con cols=165 lines=40");
#endif

	CP_Main( /* NOW GAME START! \^o^/ */ );
	
	return EXIT_SUCCESS;
}
