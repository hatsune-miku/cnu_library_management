
/*
	Name:			CoreUI.c
	Description:	Implementation of CoreUI.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdarg.h>
#include <locale.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h> // getch
#else
#include <curses.h> // getch
#define _getch getch
#endif

#include "CoreUI.h"
#include "CoreSecure.h"
#include "BookAttr.h"
#include "CoreLibrary.h"
#include "AlgorithmUtil.h"

// for GUI in Windows.
#ifdef _WIN32

#include <Windows.h>
HWND global_hwnds_label[CP_COND_NAME_SIZE];
HWND global_hwnds_textview[CP_COND_NAME_SIZE];
HWND whWnd, button;
UINT global_hwndcnt = 0;

char global_bookattr[CP_COND_NAME_SIZE][CP_COND_VALUE_SIZE];

// is changing something?
bool global_flagModify = false;


#endif

int global_total_width = 0;

void UI_Clear(void) {
	system(clearCmd());
}

void UI_InitWithUnicodeCS(void) {
	setlocale(LC_ALL, "chs");
}

void UI_PrintBookHeader(FILE* target) {
	
	fprintf(target, "       "); // padding 7 spaces

	int nattr = BA_GetNattr();
	char item[4], name[BA_NAME_SIZE], type[BA_TYPE_SIZE];
	global_total_width = 0;

	for (int i = 0; i < nattr; i++) {
		// label 'i' linked to width.
		sprintf(item, "%di", global_order[i]);

		// got it
		int width = getInt(PATH_BOOK_ATTR, item);
		global_total_width += width;

		// get attr name
		BA_GetAttr(global_order[i], name, type);

		// print attr name out
		char* pname = str(name);
		UI_Fprintf(target, pname);
		int len = wstrlen(pname);
		free(pname);

		// padding
		for (int j = 0; j < width - len; j++)
			fputc(' ', target);
	}
	fputc(' ', target);
}

// TODO: Memory problem.
char* UI_GetAttrFormat(char* name) {
	char* format_tem = (char*)malloc(UI_ATTR_FORMAT_SIZE * sizeof(char));
	sprintf(format_tem, "%sF", name);
	return format_tem;
}

void UI_AttributedInput(int index, char* name, char* target) {
	char* input_type = BA_GetLabel(index, 't');
	char* attr_format = UI_GetAttrFormat(name);

	if (!strcmp(input_type, "MENU")) {
		char** arr = UI_MenuMakeWithString(attr_format,
			BA_GetLabel(index, 'e'));

		// prevent illegal input.
		int _index;
		while ((_index = UI_Input()) > UI_MENU_HEIGHT
			|| _index < 0);

		char* pname = str(arr[_index - 1]);
		strcpy(target, (pname));
		free(pname);

		LM_FreeDynamicArray(arr, UI_MENU_HEIGHT);
		free(attr_format);
	}
	else if (!strcmp(input_type, "INPUT")
		|| !strcmp(input_type, "") // ERROR
		|| !strcmp(input_type, "RO")) {
		UI_Printf(attr_format); 
		UI_Printf(": ");
		readline(target, CP_COND_VALUE_SIZE);
		
		// readline(target);
	}
}

void UI_AddBookNavi(void) {
	putchar('\n');
	int n = BA_GetNattr(), cnt = 0;
	char name[BA_NAME_SIZE], type[BA_TYPE_SIZE];
	char** args = (char**)malloc(n * sizeof(char*));

	for (int i = 0; i < n; i++) {
		args[i] = (char*)malloc(BA_FORMAT_SIZE * sizeof(char));

		char* label = BA_GetLabel(i, 't');
		if (!strcmp(label, "RO")) {
			// read only attr. skip.
			free(label);
			continue;
		}
		free(label);

		BA_GetAttr(i, name, type);

		UI_Printf("STR_ADD_INPUT");
		UI_Printf("%s", name);

		UI_AttributedInput(i, name, args[cnt]);
		cnt++;
	}
	bool suc = LM_Add(args, cnt, true);

	for (int i = 0; i < n; i++)
		free(args[i]);
	free(args);

	if (!suc)
		UI_MessageBox("STR_ADD_ERROR", false);
}

bool UI_Prompt(const char* msg, bool tryGUI) {
	if (!tryGUI) {
		system(clearCmd());
		NEWLINE;
		LPADDING;
		UI_Printf(msg);
		NEWLINE;
		NEWLINE;
		int input = 0;
		while (true) {
			if (input != 0)
				while (getchar() != '\n');
			LPADDING;
			UI_Printf("STR_ADD_INPUT");
			printf(" (Y/N): ");
			input = getchar();
			if (input == 'Y' || input == 'y')
				return true;
			if (input == 'N' || input == 'n')
				return false;
		}
		return false;
	}
	else {
#ifdef _WIN32
		char* pmsg = str(msg);
		int ret = MessageBox(NULL, pmsg, "Prompt", MB_ICONINFORMATION + MB_YESNO);
		return ret == IDYES;
#endif
		return false;
	}
}

void UI_MessageBoxWithType(const char* msg, const char* caption, bool tryGUI, unsigned int type) {
#ifdef _WIN32
	if (tryGUI) {
		char* pmsg = str(msg);
		char* pcaption = str(caption);
		if (!strcmp(pmsg, ""))
			MessageBox(NULL, msg, pcaption, type);
		else
			MessageBox(NULL, pmsg, pcaption, type);
		free(pmsg);
		free(pcaption);
		return;
	}
#endif 

	system(clearCmd());
	NEWLINE;
	LPADDING;
	UI_Printf(msg);
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_PRESS_ANY_KEY_TO_CONTINUE");

#ifdef _WIN32
	_getch();
#else
	initscr();
	_getch();
	endwin();
#endif
}

void UI_MessageBox(const char* msg, bool tryGUI) {
	UI_MessageBoxWithType(msg, "nya!", tryGUI, MB_ICONINFORMATION);
}

void UI_Fprintf(FILE* target, const char* format, ...) {
	char* bf = str(format);
	const char* f = bf;
	char* arg = NULL;
	char* text = NULL;

	if (!strcmp("", f)) // ERROR
		f = format;

	int val = 0;
	float fv = 0;

	va_list ap;
	va_start(ap, format);

	while (*f != '\0') {
		if (*f == '/' || *f == '%') {
			switch (*(f + 1)) {
			case 't':
				fputc('\t', target);
				break;
			case 'n':
				fputc('\n', target);
				break;
			case 's':
				arg = va_arg(ap, char*);
				text = str(arg);
				if (!strcmp("", text)) // ERROR
					fprintf(target, "%s", arg);
				else
					fprintf(target, "%s", text);
				free(text);
				break;
			case 'c':
				fputc(va_arg(ap, char), target);
				break;
			case 'd':
				val = va_arg(ap, int);
				fprintf(target, "%d", val);
				break;
			case 'f':
				fv = va_arg(ap, float);
				fprintf(target, "%f", fv);
				break;
			}
			f++;
		}
		else
			fprintf(target, "%c", *f);
		// putchar(*f);
		f++;
	}
	va_end(ap);
	free(bf);
}

void UI_Printf(const char* format, ...) {
	char* bf = str(format);
	const char* f = bf;
	char* arg = NULL;
	char* text = NULL;

	if (!strcmp("", f) && !strstr(format, "STR_")) // raw input
		f = format;

	int val = 0;
	float fv = 0;

	va_list ap;
	va_start(ap, format);

	while (*f != '\0') {
		if (*f == '/' || *f == '%') {
			switch (*(f + 1)) {
			case 't':
				putchar('\t');
				break;
			case 'n':
				putchar('\n');
				break;
			case 's':
				arg = va_arg(ap, char*);
				text = str(arg);
				if (!strcmp("", text)) // not found. raw print
					printf("%s", arg);
				else
					printf("%s", text);
				free(text);
				break;
			case 'c':
				putchar(va_arg(ap, char));
				break;
			case 'd':
				val = va_arg(ap, int);
				printf("%d", val);
				break;
			case 'f':
				fv = va_arg(ap, float);
				printf("%f", fv);
				break;
			}
			f++;
		}
		else
			printf("%c", *f);
		// putchar(*f);
		f++;
	}
	va_end(ap);
	free(bf);
}

void dbgprintf(const char* str) {
#ifdef _DEBUG
	printf("%s\n", str);
#else
	UNREFERENCED_PARAMETER(str);
#endif
}

const char* clearCmd(void) {
#ifdef _WIN32
	return "cls"; /* Windows */
#else
	return "clear"; /* Linux etc. */
#endif
}

const char* lsCmd(void) {
#ifdef _WIN32
	return "dir /b "; /* Windows */
#else
	return "ls"; /* Linux etc. */
#endif
}

int UI_Input(void) {
	char str[CP_COND_VALUE_SIZE / 4] = { '\0' };
	UI_Printf("%s", "IN");
	readline(str, CP_COND_VALUE_SIZE / 4);
	// readline(str);
	int ret = advatoi(str);
	return ret;
}

/*
void UI_TableMakeWithArray(
	const char* title,
	int formatNum,
	int num,
	const char* formatArr,
	const char* arr,
	int subNumber) {

	if (formatNum < 2)
		return;

	printf("\n\n");
	UI_Printf("STR_MENU_TITLE_TEMPLATE", title);
	printf("\n\n");

	bool flagNoNumber = false;
	if (subNumber == -1)
		flagNoNumber = true;

	for (int i = 0; i < formatNum; i++)
		if(i == 1)
			printf("%-16s")
		else
			printf("-16s")

	for (int i = 0; i < num; i++) {
		if (!flagNoNumber)
			printf("%d. ", subNumber + i);
		printf("%s\n", arr[i]);
	}
}
*/
// deprecated.

#ifdef _WIN32

// _UI_GatherBookInfo(bool);
// Usage: button's OnClick() event.

void _UI_GatherBookInfo(bool flagModify) {
	char** newValues = LM_DynamicArray2Make(CP_COND_VALUE_SIZE, global_hwndcnt);

	if (flagModify) {
		CP_Condition cond[1];
		char** modifiedItems = LM_DynamicArray2Make(CP_COND_VALUE_SIZE, global_hwndcnt);

		for (UINT i = 0; i < global_hwndcnt; i++) {
			strcpy(modifiedItems[i], global_bookattr[i]);
			GetWindowText(global_hwnds_textview[i], newValues[i], CP_COND_VALUE_SIZE);
		}

		// condition make
		strcpy(cond[0].name, "STR_BOOK_NUMBER");
		GetWindowText(global_hwnds_textview[0],
			cond[0].value, CP_COND_VALUE_SIZE);
		cond[0].cond = '=';

		// perform change
		char exec[CP_EXEC_SIZE];
		LM_UpdateExecMakeWithConds(1, cond, global_hwndcnt, modifiedItems, newValues, exec);
		LM_SQLExec(sql, exec, NULL, NULL, NULL);

		LM_FreeDynamicArray(modifiedItems, global_hwndcnt);
	}
	else {
		// gather info
		for (UINT i = 0; i < global_hwndcnt; i++)
			GetWindowText(global_hwnds_textview[i], newValues[i], CP_COND_VALUE_SIZE);

		// perform add
		bool suc = LM_Add(newValues, global_hwndcnt, false);
		
		if (!suc)
			UI_MessageBox("STR_ADD_ERROR", false);
	}

	LM_FreeDynamicArray(newValues, global_hwndcnt);
}

LRESULT CALLBACK WindowProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {

	UINT flagConfirmed = 0;
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
	{
		UINT nID = LOWORD(wParam);
		UINT nEvent = HIWORD(wParam);
		// HWND rhwnd = (HWND)lParam;
		// Not used.
		UINT index = nID - 1020; // just a id not hwnd
		if ((nEvent == EN_SETFOCUS
			|| nEvent == CBN_SETFOCUS)
			&& index < global_hwndcnt)
			for (UINT i = 0; i < global_hwndcnt; i++) {
				if (i == index)
					SendMessage(global_hwnds_textview[i], EM_SETSEL, 0, -1);
				else
					SendMessage(global_hwnds_textview[i], EM_SETSEL, 0, 0);
				UpdateWindow(global_hwnds_textview[i]);
			}
		else if (nEvent == BN_CLICKED) {
			flagConfirmed = 1; // confirm
			break;
		}
		break;
	}
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) { // ESC
			flagConfirmed = 2; // cancel
			break;
		}
		if (wParam == VK_TAB) // tab. focus next.
			SetFocus(
				GetNextWindow(
					GetNextWindow(GetFocus(), GW_HWNDNEXT), GW_HWNDNEXT));
		// bypass a label.
		else if (wParam == VK_RETURN) {
			flagConfirmed = 1; // confirm
			break;
		}
		break;
	}

	if (flagConfirmed) {
		if (flagConfirmed == 1)
			_UI_GatherBookInfo(global_flagModify);
		DestroyWindow(whWnd);
		PostQuitMessage(0);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif

// set bookNumber = NULL if add.
bool UI_BookModifyWithGUI(char* bookNumber) {

#ifdef _WIN32

	// Windows. Using GUI.

	global_hwndcnt = 0;

	char** result;
	int row, col;
	int cnattr = BA_GetCreateNattr();

	if (bookNumber) {
		LM_QueryBookWithBookNumber(
			bookNumber, &row, &col, &result);
		if (row == 0)
			return true;
	}
	else {
		result = LM_DynamicArray2Make(
			CP_COND_VALUE_SIZE, CP_COND_NAME_SIZE);
		for (int i = 0; i < CP_COND_NAME_SIZE; i++)
			strcpy(result[i], "");
		row = col = 0;
	}

	int window_height = (cnattr - 4) * 45 + 200;

	/*
	for (int i = 1; i <= nattr; i++)
		printf("%s ", result[nattr + i]);
	*/

	// declare private method. (force cpp :/
	LRESULT CALLBACK WindowProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	// pending free!
	TCHAR className[] = TEXT("BookModifyGUISolution");
	char* button_confirm = str("STR_BUTTON_CONFIRM");
	char* window_caption = NULL;
	if (bookNumber)
		window_caption = str("STR_LIBMAN_MODIFYONE");
	else
		window_caption = str("STR_BOOK_ADD");

	HINSTANCE hInstance = GetModuleHandle(NULL);
	
	// wndclass make
	WNDCLASS wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.lpszClassName = className;
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;

	static BOOL registered = FALSE;

	if (!registered) {
		if (!RegisterClass(&wndClass)) {
			// GUI not avilable. fall back to CLI.
			registered = FALSE;
			return false;
		}
		else
			registered = TRUE;
	}

	HFONT font = CreateFont(
		18, // nHeight
		0, // nWidth
		0, // nEscapement
		0, // nOrientation
		FW_NORMAL, // nWeight
		FALSE, // bItalic
		FALSE, // bUnderline
		0, // cStrikeOut
		ANSI_CHARSET, // nCharSet
		OUT_DEFAULT_PRECIS, // nOutPrecision
		CLIP_DEFAULT_PRECIS, // nClipPrecision
		DEFAULT_QUALITY, // nQuality
		DEFAULT_PITCH | FF_SWISS,
		"Microsoft YaHei" // nPitchAndFamily
	);

	// main window
	whWnd = CreateWindow(
		className, // class name
		(window_caption), // caption
		WS_CAPTION | WS_SYSMENU, // style
		400, // pos_x
		250, // pos_y
		380, // width
		window_height, // height
		NULL,
		NULL,
		hInstance,
		NULL);

	char name[CP_COND_NAME_SIZE];
	char* cpname;
	for (int i = 0; i < cnattr - 1; i++) {
		char* label = BA_GetLabel(global_order[global_hwndcnt], 't');
		// read only attr. skip.
		if (!strcmp(label, "RO"))
			continue;

		BA_GetAttr(global_order[i], name, NULL);
		char* pname = str(name);
		cpname = pname;

		global_hwnds_label[global_hwndcnt] =
			CreateWindow(
				TEXT("STATIC"), // class name
				TEXT(""), // caption
				WS_VISIBLE | WS_CHILD, // style
				15, // pos_x
				15 + global_hwndcnt * 45, // pos_y
				80, // width
				24, // height
				whWnd,
				NULL, // id
				hInstance,
				NULL);
		SetWindowText(global_hwnds_label[global_hwndcnt],
			cpname);
		if (!strcmp(label, "INPUT")) {
			global_hwnds_textview[global_hwndcnt] =
				CreateWindow(
					TEXT("EDIT"),
					TEXT(""),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					115,         // x
					15 + global_hwndcnt * 45, // y
					220,        // width
					24,        // height
					whWnd,       // parent
					(HMENU)(1020 + i),       // menu
					hInstance,
					NULL);
		}
		else { // MENU
			global_hwnds_textview[global_hwndcnt] =
				CreateWindow(
					TEXT("COMBOBOX"),
					TEXT(""),
					WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
					115,         // x
					15 + global_hwndcnt * 45, // y
					220,        // width
					24,        // height
					whWnd,       // parent
					(HMENU)(1020 + i),       // menu
					hInstance,
					NULL);
			
			char* list = BA_GetLabel(global_order[global_hwndcnt], 'e'); // list of strs
			char** item = LM_DynamicArray2Make(8 * CP_COND_SIZE, CP_COND_NAME_SIZE); 
			int n;
			
			split(list, ',', item, &n);
			
			SendMessage(global_hwnds_textview[global_hwndcnt], CB_RESETCONTENT, 0, 0);

			for (int j = 0; j < n; j++) {
				char* pname2 = str(item[j]);
				char* cname = (pname2); // parse real name

				// add to combobox
				SendMessage(global_hwnds_textview[global_hwndcnt],
					CB_ADDSTRING, (WPARAM)0, (LPARAM)cname);

				char* pg = U2G(result[cnattr + global_hwndcnt + 2]);
				if (!strcmp(cname, pg))
					SendMessage(global_hwnds_textview[global_hwndcnt],
						CB_SETCURSEL, j, 0); // select
				free(pg);
				free(pname2);
			}

			if (!bookNumber)
				SendMessage(global_hwnds_textview[global_hwndcnt],
					CB_SETCURSEL, 0, 0); // if !bookNumber select nothing
			LM_FreeDynamicArray(item, CP_COND_NAME_SIZE);
		}
		free(label);
		// set font
		SendMessage(global_hwnds_textview[global_hwndcnt],
			WM_SETFONT, (WPARAM)font, 1);
		SendMessage(global_hwnds_label[global_hwndcnt],
			WM_SETFONT, (WPARAM)font, 1);
		strcpy(global_bookattr[global_hwndcnt], (name));

		if (bookNumber) {
			char* pg = U2G(result[cnattr + global_hwndcnt + 2]);
			SetWindowText(global_hwnds_textview[global_hwndcnt],
				pg);
			free(pg);
		}
		else {
			char namef[CP_COND_VALUE_SIZE];
			strcpy(namef, name);
			strcat(namef, "F");
			char* pnamef = str(namef);
			if (strcmp(pnamef, "")) // set text for textbox
				SetWindowText(
					global_hwnds_textview[global_hwndcnt],
					(pnamef));
			free(pnamef);
		}

		global_hwndcnt++;
		free(pname);
	}
	button = CreateWindow(
		TEXT("BUTTON"),
		(button_confirm),
		WS_VISIBLE | WS_CHILD,
		115 + 220 - 100,         // x
		window_height - 30 - 15 - 45, // y
		100,        // width
		30,        // height
		whWnd,       // parent
		NULL,       // menu
		hInstance,
		NULL);

	global_flagModify = bookNumber != NULL;

	ShowWindow(whWnd, SW_SHOW);

	SendMessage(whWnd, WM_SETFONT, (WPARAM)font, 1);
	SendMessage(button, WM_SETFONT, (WPARAM)font, 1); // set font

	UpdateWindow(whWnd);
	if (global_hwndcnt)
		SetFocus(global_hwnds_textview[0]);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		WindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam); // merged wndproc.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	free(window_caption);
	free(button_confirm);
	if (bookNumber)
		sqlite3_free_table(result);
	else
		LM_FreeDynamicArray(result, CP_COND_NAME_SIZE);
	return true;
#else
	return false;
#endif
}


void UI_MastercodeInputWithCustomMsg(char* msgTag, char* uPassword) {
	// char* msg = str(msgTag);
	// Not used.
	(msgTag); // unferenced.
	 
	int ch, cnt = 0;
	while ((ch = _getch()) != '\r'
		&& cnt < CP_COND_VALUE_SIZE - 1) {
		if (ch == '\b') {
			if (cnt > 0) {
				uPassword[--cnt] = '\0';
				for (int i = 0; i < sizeof(char); i++) {
					putchar(ch);
					putchar(' ');
					putchar(ch);
				}
			}
		}
		else if (0 < ch && ch < 128) {
			uPassword[cnt++] = (char)ch;
			putchar('*');
			// nya!
		}
	}
	putchar('\n');
	uPassword[cnt] = '\0';
	// free(msg);
}


void UI_MastercodeInputSolution(char* uPassword, bool tryGUI) {
	UNREFERENCED_PARAMETER(tryGUI);
	UI_MastercodeInputWithCustomMsg("STR_SETTINGS_PASSWD_RAWTIP", uPassword);
}

bool UI_MastercodeCheck(bool flagFullScreen) {
	bool flagNoPasswd = SM_MasterCodeCheck(NULL);
	if (flagNoPasswd)
		return true;

	char code[CP_COND_VALUE_SIZE];

	if (flagFullScreen)
		system(clearCmd());
	
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_SETTINGS_PASSWD_TIP");
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_ADD_INPUT");
	UI_Printf("STR_DATASECURE_PASSWD_NAME");
	UI_Printf(": ");

	UI_MastercodeInputSolution(code, true);

	if (!strcmp(code, "forgot"))
		if (!SM_SQAnswerCheck(NULL))
			if (UI_SQCheck())
				SM_MasterCodePush("");
			else
				return false;
		else {
			UI_MessageBox("STR_SETTINGS_SQ_NOTSET", false);
			return false;
		}
		bool flagCorrect = SM_MasterCodeCheck(code);
		if (!flagCorrect)
			UI_MessageBox("STR_DATASECURE_WRONG_PASSWD", false);

		return flagCorrect;
}

bool UI_SQCheck(void) {
	bool flagNoSQ = SM_SQAnswerCheck(NULL);
	if (flagNoSQ)
		return true;

	char sQuestion[CP_COND_VALUE_SIZE];
	char uAnswer[CP_COND_VALUE_SIZE];
	SM_SQQuestionGet(sQuestion);

	system(clearCmd());
	NEWLINE;
	LPADDING;
	UI_Printf("STR_SETTINGS_SQ_TIP");
	NEWLINE;
	NEWLINE;
	LPADDING;
	printf("%s\n\n  > ", sQuestion);
	// readline(uAnswer);
	readline(uAnswer, CP_COND_VALUE_SIZE);

	bool flagCorrect = SM_SQAnswerCheck(uAnswer);
	if (!flagCorrect)
		UI_MessageBox("STR_DATASECURE_WRONG_SQ", false);

	return flagCorrect;
}

void UI_SettingsMake(const char* name, const char* intro, const char* key, bool InputOrSwitch) {
	system(clearCmd());

	NEWLINE;
	LPADDING;
	UI_Printf(intro);
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_CHECK_QUESTION_CURRENT");

	int input;

	if (InputOrSwitch) {
		// Input
		char* value = config(PATH_CONFIG, key);
		UI_Printf("%s", value);
		UI_MenuMake(name, 2,
			"STR_SETTINGS_STAT_MODIFY",
			"STR_SETTINGS_STAT_JUSTKEEP");

		do {
			input = UI_Input();
		} while (input < 1 || input > 2);

		if (input == 1) {
			char newValue[CP_COND_VALUE_SIZE];
			NEWLINE;
			LPADDING;
			UI_Printf("STR_ADD_INPUT");
			UI_Printf(name);
			printf(": ");
			// readline(newValue);
			readline(newValue, CP_COND_VALUE_SIZE);
			configWrite(PATH_CONFIG, key, newValue);
		}
	}
	else {
		// switch
		int stat = getInt(PATH_CONFIG, key);
		if (stat)
			UI_Printf("STR_SETTINGS_STAT_ENABLED");
		else
			UI_Printf("STR_SETTINGS_STAT_DISABLED");

		UI_MenuMake(name, 2,
			stat ?
			"STR_SETTINGS_STAT_KEEP_ENABLED" :
			"STR_SETTINGS_STAT_ENABLED",
			stat ? "STR_SETTINGS_STAT_DISABLED" :
			"STR_SETTINGS_STAT_KEEP_DISABLED");

		do {
			input = UI_Input();
		} while (input < 1 || input > 2);

		if (input == 1) {
			if (!stat)
				setInt(PATH_CONFIG, key, 1);
			return;
		}
		else {
			if (stat)
				setInt(PATH_CONFIG, key, 0);
			return;
		}
	}
}

void UI_SwitchMake(const char* name, const char* intro, const char* key) {
	UI_SettingsMake(name, intro, key, false);
}

void UI_ListMakeWithArray(
	const char* title,
	int num,
	const char** arr,
	int subNumber) {

	NEWLINE;
	NEWLINE;
	UI_Printf("STR_MENU_TITLE_TEMPLATE", title);
	NEWLINE;
	NEWLINE;

	bool flagNoNumber = false;
	if (subNumber == -1)
		flagNoNumber = true;
	for (int i = 0; i < num; i++) {
		if (!flagNoNumber)
			printf("%d. ", subNumber + i);
		printf("%s\n", arr[i]);
	}
}

// set title = NULL for raw print.
void UI_ListMakeWithSqlite3Array(
	const char* title,
	void(*HeaderProcessor)(FILE*),
	int nrow,
	int ncol,
	bool flagShowDeleted,
	int base,
	char** arr,
	void (*callback)(char*, FILE*),
	FILE* target) {

	if (title) {
		fprintf(target, "\n\n");
		UI_Fprintf(target, "STR_MENU_TITLE_TEMPLATE", title);
		fprintf(target, "\n\n");

		if (HeaderProcessor)
			HeaderProcessor(target);

		for (int i = 0; i < 13; i++)
			fputc(' ', target);
		for (int i = 0; i < global_total_width + 15; i++)
			fputc('-', target);
		fputc('\n', target);

		// TODO: solve memory issue
	}

	int n = ncol;
	const int npreserved = 2;
	for (int i = 0, cnt = 0; i < nrow; i++) {
		n++;
		if (!flagShowDeleted
			&& !strcmp(arr[n + ncol - npreserved], "1")) { // deleted
			n += ncol - npreserved;
		}
		else {
			fprintf(target, " %4d. ", cnt + 1 + base); // number
			cnt++;
			char bookNumber[CP_COND_VALUE_SIZE];
			for (int j = 0; j < ncol - npreserved; j++) {
				char booki[8], type[CP_COND_VALUE_SIZE / 4];
				sprintf(booki, "%di", global_order[j]); // info
				BA_GetAttr(global_order[j], NULL, type);
				if (strstr(type, "UNIQUE")) {
					// found STR_BOOK_NUMBER
					strcpy(bookNumber, arr[n]);
				}
				int width = getInt(PATH_BOOK_ATTR, booki);
				int len = wstrlen(arr[n]);
                char utf8[CU_BUFFER_SIZE];
                ascii2Utf8(arr[n], len, utf8);
				fprintf(target, "%s", utf8);
				
				int realLen = wstrlen(utf8);
				for (int k = 0; k < width - realLen; k++)
					fputc(' ', target);
				n++;
			}
			if (callback)
				callback(bookNumber, target);
			fputc('\n', target);
		}
		n += 1; // for STR_PRESERVED_DELETED
	}
}

// TODO: memory problem.
char** UI_MenuMakeWithString(const char* title, const char* str) {
	char** arr = (char**)malloc(UI_MENU_HEIGHT * sizeof(char*));
	int n = 0;
	for (int i = 0; i < UI_MENU_HEIGHT; i++)
		arr[i] = (char*)malloc(UI_MENU_WIDTH * sizeof(char));
	split(str, ',', arr, &n);
	UI_MenuMakeWithArray(title, n, arr);
	return arr;
}

void UI_MenuMakeWithArray(const char* title, int num, const char** arr) {
	printf("\n\n");
	UI_Printf("STR_MENU_TITLE_TEMPLATE", title);
	printf("\n\n");
	for (int i = 0; i < num; i++) {
		char* text = str(arr[i]);
		printf("\t%2d. %s\n", i + 1, text);
		free(text);
	}
}

void UI_MenuMake(const char* title, int num, ...) {
	NEWLINE;
	NEWLINE;
	UI_Printf("STR_MENU_TITLE_TEMPLATE", title);
	NEWLINE;
	NEWLINE;
	va_list ap;
	va_start(ap, num);

	// va_arg(ap, int); // crashes in Release.

	for (int i = 0; i < num; i++) {
		const char* raw = va_arg(ap, char*);
		char* text = str(raw);
		UI_Printf("/t%d. %s/n", i + 1, text);
		free(text);
	}
	va_end(ap);
}
