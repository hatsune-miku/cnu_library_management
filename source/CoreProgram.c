
/*
	Name:			CoreProgram.c
	Description:	Implementaion of CoreProgram.c
*/

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>

#include "CoreProgram.h"
#include "AlgorithmUtil.h"
#include "BookAttr.h"
#include "CoreSecure.h"
#include "CoreLibrary.h"
#include "CoreNetwork.h"

#define CP_FUNCTION_ADDITION int input,pass=0
#define CLEAR_SCREEN system(clearCmd())

// -+-+-+-+-+-+-+-+-+-+-+- global var -+-+-+-+-+-+-+-+-+-+-+-+-+-

/* book filter */
CP_Condition	global_filter_condition[CP_COND_SIZE];
int				global_condition_cnt = 0;

/* buy mode */
typedef struct {
	char		number[CP_COND_VALUE_SIZE];
	float		price;
	float		discount;
	int			count;
	void*		preserved;
} good;

CP_Condition	global_conds[CP_COND_VALUE_SIZE];
good			global_goods[CP_COND_VALUE_SIZE];
int				global_nbook = 0;

/* search */
int				global_nkeyword = 0;
char**			global_keywords = NULL;

// -+-+-+-+-+-+-+-+-+-+-+- global var -+-+-+-+-+-+-+-+-+-+-+-+-+-

void CP_Main(void) {
	CP_FUNCTION_ADDITION;

	while (true) {
		CLEAR_SCREEN;
		UI_Printf("TITLE_TEMPLATE", "APP_NAME", "CREDIT");
		UI_MenuMake("STR_MAIN_MENU", 9,
			"STR_BOOK_LOOKUP",
			"STR_BOOK_ADD",
			"STR_BOOK_MAN",
			"STR_BOOK_CLOUD",
			"STR_BUYMODE",
			"STR_SETTINGS",
			"STR_ANY_SQLITE_CMD",
			"STR_ABOUT",
			"STR_EXIT");

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // Book lookup
				CP_BookManage();
				break;
			case 2: // Book add
				CP_AddBook();
				break;
			case 3: // Borrow Mode
				CP_BorrowMode();
				break;
			case 4: // Cloud service
				CP_CloudService();
				break;
			case 5: // buy mode
				CP_BuyMode();
				break;
			case 6: // Settings
				CP_Settings();
				break;
			case 7: // Any Sqlite cmd
				CP_AnySqlCmd();
				break;
			case 8: // About
				CP_OpenSourceLic();
				break;
			case 9: // Exit
				sqlite3_close(sql);
				return;
			default:
				pass = 0;
			}
		}
		pass = 0;
	}
}

void _CP_OpenSourceLicCallback(char* line) {
	LPADDING;
	printf("%s", line);
}

void CP_OpenSourceLic(void) {
	CP_FUNCTION_ADDITION;

	CLEAR_SCREEN;
	UI_MenuMake("STR_OSL_TIP", 5,
		"STR_SOFTWARE_NAME_SQLITE",
		"STR_SOFTWARE_NAME_LIBCURL",
		"STR_SOFTWARE_NAME_OPENSSL",
		"STR_OSL_APACHEV2",
		"STR_RETURN");
	while (!pass) {
		pass = 1;
		switch (input = UI_Input()) {
		case 1: // sqlite
			massRead(LIC_PATH_SQLITE, _CP_OpenSourceLicCallback);
			break;
		case 2: // libcurl
			massRead(LIC_PATH_LIBCURL, _CP_OpenSourceLicCallback);
			break;
		case 3: // openssl
			massRead(LIC_PATH_OPENSSL, _CP_OpenSourceLicCallback);
			break;
		case 4: // apachev2
			massRead(LIC_PATH_APACHEV2, _CP_OpenSourceLicCallback);
			break;
		case 5: // return.
			return;
		default:
			pass = 0;
		}
	}
	getchar();
	pass = 0;
}

void CP_CloudService(void) {
	CP_FUNCTION_ADDITION;

	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(true))
		return;

	CLEAR_SCREEN;

	bool flagSucceed = 0; 

	NEWLINE;
	LPADDING;
	UI_Printf("STR_CLOUD_KEYTIP");
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_CLOUD_INPUT_KEY");

	// user code input
	char userCode[CP_COND_VALUE_SIZE];
	readline(userCode, CP_COND_VALUE_SIZE);

	char filename_db[CP_COND_VALUE_SIZE];
	char filename_bookattr[CP_COND_VALUE_SIZE];
	char filename_config[CP_COND_VALUE_SIZE];

	// filenames gen
	sprintf(filename_db, "%s.dbstorage", userCode);
	sprintf(filename_bookattr, "%s.bookattr", userCode);
	sprintf(filename_config, "%s.config", userCode);

	UI_MenuMake("STR_BOOK_CLOUD", 3,
		"STR_CLOUD_UPLOAD",
		"STR_CLOUD_DOWNLOAD",
		"STR_RETURN");

	while (!pass) {
		pass = 1;
		switch (input = UI_Input()) {
		case 1: // upload
			sqlite3_close(sql);
			LPADDING;
			UI_Printf("STR_CLOUD_LOADING");
			flagSucceed = NU_FileUpload(filename_db, PATH_DATABASE)
				&& NU_FileUpload(filename_bookattr, PATH_BOOK_ATTRA)
				&& NU_FileUpload(filename_config, PATH_CONFIGA);
			break;
		case 2: // download
			sqlite3_close(sql);
			LPADDING;
			UI_Printf("STR_CLOUD_LOADING");
			flagSucceed = NU_FileDownload(filename_db, PATH_DATABASE)
				&& NU_FileDownload(filename_bookattr, PATH_BOOK_ATTRA)
				&& NU_FileDownload(filename_config, PATH_CONFIGA);
			break;
		case 3: // return.
			return;
		default:
			pass = 0;
		}
	}
	pass = 0;

	if (flagSucceed)
		UI_MessageBox("STR_CLOUD_FINISH", false);
	else
		UI_MessageBox("STR_CLOUD_FAIL", false);

	LM_Init();
}

void CP_AddBook(void) {
	if (getInt(PATH_CONFIG, "trygui"))
		if (UI_BookModifyWithGUI(NULL))
			return;
	UI_AddBookNavi();
}

int CP_AnySqlCmdCallback(void* NotUsed, int argc, char **argv, char **argn) {
	UNREFERENCED_PARAMETER(argn);
	UNREFERENCED_PARAMETER(NotUsed);

	int i;
	LPADDING;
	for (i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	putchar('\n');
	return 0;
}

void CP_AnySqlCmd(void) {
	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(true))
		return;

	char exec[CP_EXEC_SIZE];
	CLEAR_SCREEN;

	putchar('\n');
	UI_Printf("STR_CUSTOMSQL_TIP");
	while (true) {
		UI_Printf("/n > ");
		readline(exec, CP_EXEC_SIZE);
		if (!strcmp(exec, ".finish"))
			break;
		LM_SQLExec(sql, exec, CP_AnySqlCmdCallback, NULL, NULL);
	}
}


void CP_BorrowMode(void) {
	while (true) {
		CLEAR_SCREEN;

		NEWLINE;
		UI_Printf("STR_BORROWMODE_TIP");

		char exec[CP_EXEC_SIZE];
		char number[CP_COND_VALUE_SIZE];
		char** data;
		int nrow = 0, ncol = 0;

		NEWLINE;
		LPADDING;
		UI_Printf("STR_BOOK_NUMBER");
		UI_Printf(" > ");
		readline(number, CP_COND_VALUE_SIZE);

		// finish
		if (!strcmp(number, ".finish"))
			break;

		// condition make
		CP_Condition cond[1];
		strcpy(cond[0].name, "STR_BOOK_NUMBER");
		strcpy(cond[0].value, number);
		cond[0].cond = '=';

		LM_SelectConditionExecMake(1, cond, exec, -1, 0, false);
		LM_SQLQuery(sql, exec, &data, &nrow, &ncol, NULL);

		// not found
		if (ncol == 0)
			continue;

		// 1~1th data
		char title[CP_COND_VALUE_SIZE];
		char* titleTemplate = str("STR_LIBMAN_PAGE");
		sprintf(title, titleTemplate, 1, 1);
		free(titleTemplate);
		UI_ListMakeWithSqlite3Array(title, UI_PrintBookHeader, nrow, ncol, false, 0, data, NULL, stdout);

		// advice
		UI_MenuMake("STR_BORROWMODE_ADVICE_TIP", 6,
			"STR_ADVICE_5",
			"STR_ADVICE_4",
			"STR_ADVICE_3",
			"STR_ADVICE_2",
			"STR_ADVICE_1",
			"STR_ADVICE_0");
		int input, rank = -1;
		do {
			input = UI_Input();
		} while (input > 6 || input < 1);
		rank = 6 - input;

		// give up
		if (rank == 3)
			continue;

		// new value make
		char addition[CP_EXEC_SIZE];
		char* label = BA_NameForLabel("STR_BOOK_ADVICE_LEVEL", 'v');
		sprintf(addition, label, rank);
		free(label);

		char** modifiedItems = LM_DynamicArray2Make(CP_COND_NAME_SIZE, 1);
		char** newValues = LM_DynamicArray2Make(CP_EXEC_SIZE, 1);

		strcpy(modifiedItems[0], "STR_BOOK_BORROW_COUNT");
		strcpy(newValues[0], "STR_BOOK_BORROW_COUNT + 1");

		// borrow cnt + 1
		LM_UpdateExecMakeWithConds(1, cond, 1, modifiedItems, newValues, exec);
		LM_SQLExec(sql, exec, CP_AnySqlCmdCallback, NULL, NULL);

		strcpy(modifiedItems[0], "STR_BOOK_ADVICE_LEVEL");
		strcpy(newValues[0], addition);

		// update advice level
		LM_UpdateExecMakeWithConds(1, cond, 1, modifiedItems, newValues, exec);
		LM_SQLExec(sql, exec, CP_AnySqlCmdCallback, NULL, NULL);

		LM_FreeDynamicArray(modifiedItems, 1);
		LM_FreeDynamicArray(newValues, 1);
	}
	// CP_Main();
}

void CP_ConditionMake(CP_Condition* target, const char* name, const char op, const char* value) {
	strcpy(target->name, name);
	strcpy(target->value, value);
	target->cond = op;
}

void _CP_BuyModeCallback(char* bookNumber, FILE* target) {
	char* _template = str("STR_BM_ADDITION_TEMPLATE");
	char format[CP_COND_VALUE_SIZE];

	for (int i = 0; i < global_nbook; i++) {
		if (!strcmp(global_goods[i].number, bookNumber)) {
			// found
			sprintf(format,
				_template,
				global_goods[i].count,
				global_goods[i].price * global_goods[i].count * global_goods[i].discount);
			UI_Fprintf(target, "\n\n  %s\n\n", format);
			break;
		}
	}
}

void CP_BuyModeCheck(void) {
	CP_FUNCTION_ADDITION;

	float total = 0;
	for (int i = 0; i < global_nbook; i++) {
		total += global_goods[i].price
			* global_goods[i].count
			* global_goods[i].discount;
	}

	char* _template = str("STR_BM_CHECK_TOTAL");
	char format[CP_FORMAT_SIZE];
	sprintf(format, _template, total);
	free(_template);
	
	UI_MenuMake(format, 2,
		"STR_BM_CHECK_FINISH",
		"STR_BM_ABANDON");

	while (!pass) {
		pass = 1;
		switch (input = UI_Input()) {
		case 1: // finish
		{
			char** modifiedValues = LM_DynamicArray2Make(CP_COND_NAME_SIZE, 1);
			char** newValues = LM_DynamicArray2Make(CP_COND_VALUE_SIZE, 1);
			char exec[CP_EXEC_SIZE];

			for (int i = 0; i < global_nbook; i++) {
				strcpy(modifiedValues[0], "STR_BOOK_STORAGE");
				sprintf(newValues[0], "STR_BOOK_STORAGE - %d", global_goods[i].count);
				CP_Condition cond[1];
				CP_ConditionMake(cond, "STR_BOOK_NUMBER", '=', global_goods[i].number);
				LM_UpdateExecMakeWithConds(1, cond, 1, modifiedValues, newValues, exec);
				LM_SQLExec(sql, exec, NULL, NULL, NULL);
			}
			global_nbook = 0;
			LM_FreeDynamicArray(modifiedValues, 1);
			LM_FreeDynamicArray(newValues, 1);
			break;
		}
		case 2: // abandon
			global_nbook = 0;
			break;
		default:
			pass = 0;
		}
	}
	pass = 0;
}

void CP_BuyMode(void) {
	CP_FUNCTION_ADDITION;

	// 64 books at most.
	char exec[CP_EXEC_SIZE];

	while (true) {
		char** data;
		int nrow, ncol;
		CLEAR_SCREEN;

		if (global_nbook > 0) {
			LM_SelectConditionExecMake(global_nbook, global_conds, exec, -1, 0, false);
			LM_SQLQuery(sql, exec, &data, &nrow, &ncol, NULL);
			UI_ListMakeWithSqlite3Array("STR_BM_CONTENT", UI_PrintBookHeader, nrow,
				ncol, false, 0, data, _CP_BuyModeCallback, stdout);
			sqlite3_free_table(data);
		}
		else {
			// no books. header only.
			UI_ListMakeWithSqlite3Array("STR_BM_CONTENT", UI_PrintBookHeader, 0, 0, false, 0, NULL, NULL, stdout);
		}
		
		UI_MenuMake("STR_BUYMODE", 5,
			"STR_BM_ADD",
			"STR_BM_REMOVE",
			"STR_BM_CHECK",
			"STR_BM_ABANDON",
			"STR_RETURN");

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // add
			{
				char bookNumber[CP_COND_VALUE_SIZE],
					count[CP_COND_VALUE_SIZE / 4] = { '\0' };

				LPADDING;
				UI_Printf("STR_ADD_INPUT");
				UI_Printf("STR_BOOK_NUMBER");
				UI_Printf(": ");
				readline(bookNumber, CP_COND_VALUE_SIZE);

				// exists?
				CP_Condition cond[1];
				char* sstorage;
				LM_ValueGetWithBookNumber(bookNumber, "STR_BOOK_STORAGE", &sstorage);
				if (!sstorage) {
					UI_MessageBox("STR_BM_NO_SUCH_BOOK", false);
					break;
				}

				// get storage
				int storage = advatoi(sstorage);
				LM_ValueGetWithBookNumberFree(&sstorage);
				CP_ConditionMake(cond, "STR_BOOK_NUMBER", '=', bookNumber);

				// get need
				LPADDING;
				UI_Printf("STR_ADD_INPUT");
				UI_Printf("STR_BM_COUNT_TIP");
				readline(count, CP_COND_VALUE_SIZE / 4);
				if (count[0] == '\0') // default: 1
					count[0] = '1';
				int need = advatoi(count);

				if (need > storage) {
					// out of storage
					char* msgTemplate = str("STR_BM_OUT_OF_STORAGE");
					char msg[CP_COND_VALUE_SIZE];
					sprintf(msg, msgTemplate, need, storage);
					free(msgTemplate);
					UI_MessageBox(msg, false);
					break;
				}

				global_goods[global_nbook].count = need;

				char* sprice;
				char* sdiscount;
				
				LM_ValueGetWithBookNumber(bookNumber, "STR_BOOK_PRICE", &sprice);
				LM_ValueGetWithBookNumber(bookNumber, "STR_BOOK_DISCOUNT", &sdiscount);

				strcpy(global_goods[global_nbook].number, bookNumber);
				global_goods[global_nbook].price = (float)atof(sprice);
				global_goods[global_nbook].discount = (float)atof(sdiscount);

				// free? don't exist
				if (global_goods[global_nbook].discount == 0)
					global_goods[global_nbook].discount = 1.0f;

				LM_ValueGetWithBookNumberFree(&sprice);
				LM_ValueGetWithBookNumberFree(&sdiscount);

				CP_ConditionCpy(global_conds + (global_nbook++), cond);
				break;
			}
			case 2: // remove
			{
				char bookNumber[CP_COND_VALUE_SIZE];
				LPADDING;
				UI_Printf("STR_ADD_INPUT");
				UI_Printf("STR_BOOK_NUMBER");
				UI_Printf(": ");
				readline(bookNumber, CP_COND_VALUE_SIZE);
				for (int i = 0; i < global_nbook; i++) {
					if (!strcmp(global_conds[i].value, bookNumber)) {
						strcpy(global_conds[i].value, "__miku!\\^o^/"); // :)
						break;
					}
				}
				break;
			}
			case 3: // check
				CP_BuyModeCheck();
				break;
			case 4: // abandon
				global_nbook = 0;
				break;
			case 5: // return
				return;
			default:
				pass = 0;
			}
		}
		pass = 0;
	}
}

void CP_AddConditionNavi(void) {
	NEWLINE;

	CP_Condition cond;
	int nattr = BA_GetNattr(), i;
	char** attr_list = (char**)malloc(CP_NATTR_COUNT * sizeof(char*));

	// list make
	for (i = 0; i < nattr; i++) {
		char name[CP_COND_SIZE];
		BA_GetAttr(i, name, NULL);
		attr_list[i] = (char*)malloc(CP_NATTR_SIZE * sizeof(char));
		strcpy(attr_list[i], name);
	}
	UI_MenuMakeWithArray("STR_FILTER_SELECT_ITEM", nattr, attr_list);

	int attr, index;
	while ((attr = UI_Input()) > nattr
		|| attr < 1);

	strcpy(cond.name, (attr_list[attr - 1]));

	UI_MenuMake("STR_FILTER_SELECT_COND", 5,
		"STR_COND_GREATER",
		"STR_COND_EQUAL",
		"STR_COND_NEQ",
		"STR_COND_LESS",
		"STR_COND_LIKE");

	while ((index = UI_Input()) > 5
		|| index < 1);

	switch (index) {
	case 1:
		cond.cond = '>';
		break;
	case 2:
		cond.cond = '=';
		break;
	case 3:
		cond.cond = 'N';
		break;
	case 4:
		cond.cond = '<';
		break;
	case 5:
		cond.cond = 'L';
		break;
	}

	UI_Printf("STR_ADD_INPUT");
	UI_Printf("STR_FILTER_INPUT_VALUE");

	// input value
	UI_AttributedInput(attr - 1, (cond.name), cond.value);

	CP_AddCondition(cond);

	LM_FreeDynamicArray(attr_list, nattr);
}

void CP_AddCondition(CP_Condition cond) {
	CP_ConditionCpy(global_filter_condition + global_condition_cnt, &cond);
	global_condition_cnt++;
}

void CP_SwitchLanguage(void) {
	CP_FUNCTION_ADDITION;

	CLEAR_SCREEN;

	NEWLINE;
	LPADDING;
	UI_Printf("STR_SETTINGS_LANGUAGEINTRO");
	NEWLINE;
	NEWLINE;

	UI_MenuMake("STR_SETTINGS_TXT_UAUALLY", 2,
		"STR_SETTINGS_LANG1",
		"STR_SETTINGS_LANG2");

	UI_Printf("STR_SETTINGS_MULTI_LANG_TIP");

	input = UI_Input();
	switchLanguage(input);
}

void CP_ResetDatabase(void) {
	if (UI_Prompt("STR_SETTINGS_RESET_CONFIRM", false)) {
		putchar('\n');
		UI_Printf("STR_SETTINGS_RESET_CHECK");
		int l = 1000;
		int r = 7; // :)
		int f = l - r, fu;
		UI_Printf("  %d - %d = ", l, r);
		scanf("%d", &fu);
		if (fu == f) {
			UI_Printf("STR_PROCESSING");
			LM_Reset(true);
		}
	}
}

void CP_SafeMode(void) {
	UI_SwitchMake(
		"STR_SETTINGS_SAFEMODE",
		"STR_SETTINGS_SAFEMODEINTRO",
		"safe_mode");
}

void CP_SetSecurityQuestion(void) {
	if (!UI_SQCheck())
		return;

	int input;

	system(clearCmd());
	NEWLINE;
	LPADDING;
	UI_Printf("STR_SETTINGS_SECURITY_QUESTIONSINTRO");
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_CHECK_QUESTION_CURRENT");

	// Input

	UI_Printf("STR_SETTINGS_SECURED");
	UI_MenuMake("STR_SETTINGS_SECURITY_QUESTIONS", 3,
		"STR_SETTINGS_STAT_MODIFY",
		"STR_SETTINGS_STAT_DELETE",
		"STR_SETTINGS_STAT_JUSTKEEP");

	do {
		input = UI_Input();
	} while (input < 1 || input > 3);

	if (input == 1) {
		char uQuestion[CP_COND_VALUE_SIZE];
		char uAnswer[CP_COND_VALUE_SIZE];

		NEWLINE;
		LPADDING;
		UI_Printf("STR_ADD_INPUT");
		UI_Printf("STR_SETTINGS_SQ_QUESTION");
		printf(": ");

		readline(uQuestion, CP_COND_VALUE_SIZE);

		LPADDING;
		UI_Printf("STR_ADD_INPUT");
		UI_Printf("STR_SETTINGS_SQ_ANSWER");
		printf(": ");

		readline(uAnswer, CP_COND_VALUE_SIZE);

		UI_Printf("STR_PROCESSING");

		SM_SQAnswerPush(uAnswer);
		SM_SQQuestionPush(uQuestion);
	}
	else if (input == 2) {
		UI_Printf("STR_PROCESSING");
		SM_SQQuestionPush("");
		SM_SQAnswerPush("");
	}
}

void CP_SetMasterCode(void) {
	if (!UI_MastercodeCheck(true))
		return;

	int input;

	CLEAR_SCREEN;

	NEWLINE;
	LPADDING;
	UI_Printf("STR_SETTINGS_MASTERCODEINTRO");
	NEWLINE;
	NEWLINE;
	LPADDING;
	UI_Printf("STR_CHECK_QUESTION_CURRENT");

	// Input

	UI_Printf("STR_SETTINGS_SECURED");
	UI_MenuMake("STR_SETTINGS_MASTERCODE", 3,
		"STR_SETTINGS_STAT_MODIFY",
		"STR_SETTINGS_STAT_DELETE",
		"STR_SETTINGS_STAT_JUSTKEEP");

	do {
		input = UI_Input();
	} while (input < 1 || input > 3);

	if (input == 1) {
		char newValue[CP_COND_VALUE_SIZE];
		char newValueConfirm[CP_COND_VALUE_SIZE];

		NEWLINE;
		LPADDING;
		UI_Printf("STR_ADD_INPUT");
		UI_Printf("STR_DATASECURE_NEWPASSWD_NAME");
		printf(": ");

		UI_MastercodeInputWithCustomMsg("STR_DATASECURE_NEWPASSWD_TIP", newValue);
		// readline(newValue);

		if (!strcmp(newValue, "forgot")) {
			UI_MessageBox("STR_SETTINGS_ILLEGAL_PWD", false);
			return;
		}

		LPADDING;
		UI_Printf("STR_ADD_REINPUT");
		UI_Printf("STR_DATASECURE_NEWPASSWD_NAME");
		printf(": ");

		UI_MastercodeInputWithCustomMsg("STR_DATASECURE_NEWPASSWDCONFIRM_TIP", newValueConfirm);
		// readline(newValueConfirm);
		if (strcmp(newValue, newValueConfirm))
			UI_MessageBox("STR_SETTINGS_CODESET_NOT_MATCH", false);
		else {
			UI_Printf("STR_PROCESSING");
			SM_MasterCodePush(newValue);
		}
	}
	else if (input == 2) {
		// delete pwd.
		UI_Printf("STR_PROCESSING");
		SM_MasterCodePush("");
	}
}

void CP_Settings(void) {
	CP_FUNCTION_ADDITION;

	// settings access requires pwd.
	if (!UI_MastercodeCheck(true))
		return;

	while (true) {
		CLEAR_SCREEN;

		UI_MenuMake("STR_SETTINGS", 8,
			"STR_SETTINGS_LANGUAGE",
			"STR_SETTINGS_SAFEMODE",
			"STR_SETTINGS_DATABASE_RESET",
			"STR_SETTINGS_MOST_BOOK_IN_ONE_PAGE",
			"STR_SETTINGS_GUI",
			"STR_SETTINGS_MASTERCODE",
			"STR_SETTINGS_SECURITY_QUESTIONS",
			"STR_RETURN");

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // language switch
				CP_SwitchLanguage();
				break;
			case 2: // safemode
				CP_SafeMode();
				break;
			case 3: // reset
				CP_ResetDatabase();
				break;
			case 4: // most book in one page
				UI_SettingsMake(
					"STR_SETTINGS_MOST_BOOK_IN_ONE_PAGE",
					"STR_SETTINGS_MOST_BOOK_IN_ONE_PAGEINTRO",
					"limit",
					true);
				break;
			case 5: // GUI
				UI_SettingsMake(
					"STR_SETTINGS_GUI",
					"STR_SETTINGS_GUIINTRO",
					"trygui",
					false);
				break;
			case 6: // master code
				CP_SetMasterCode();
				break;
			case 7: // security questions
				CP_SetSecurityQuestion();
				break;
			case 8: // return
				return;
			default:
				pass = 0;
			}
		}
		pass = 0;
	}
}

void CP_ConditionCpy(CP_Condition* dst, CP_Condition* src) {
	dst->cond = src->cond;
	strcpy(dst->name, src->name);
	strcpy(dst->value, src->value);
}

void CP_DeleteFilter(void) {
	if (global_condition_cnt == 0)
		return;

	UI_Printf("STR_FILTER_DEL_TIP");

	int input, i;
	while ((input = UI_Input()) > global_condition_cnt
		|| input < 1);

	for (i = input - 1; i < global_condition_cnt - 1; i++) {
		CP_ConditionCpy(global_filter_condition + i,
			global_filter_condition + i + 1);
	}
	global_condition_cnt--;
}

void CP_DeleteAll(bool AND) {
	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(false))
		return;

	UI_Printf("STR_PROCESSING");
	LM_DeleteBooksWithConds(global_condition_cnt, global_filter_condition, AND);
}

void CP_RemoveAll(bool AND) {

	// disabled the security check because a check
	// has been performed at CP_TrashBin entrance and
	// CP_RemoveAll is only reachable in CP_TrashBin.

	/*
	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(false))
		return;
	*/

	strcpy(global_filter_condition[global_condition_cnt].name, "PRESERVED_DELETED");
	strcpy(global_filter_condition[global_condition_cnt].value, "1");
	global_filter_condition[global_condition_cnt].cond = '=';
	global_condition_cnt++;

	UI_Printf("STR_PROCESSING");
	LM_RemoveBooksWithConds(global_condition_cnt, global_filter_condition, AND);

	global_condition_cnt--;
}

void CP_RemoveOne(void) {
	CP_Condition cond[2];

	strcpy(cond[0].name, "STR_BOOK_NUMBER");
	strcpy(cond[1].name, "PRESERVED_DELETED");
	cond[0].cond = '=';
	cond[1].cond = '=';
	strcpy(cond[1].value, "1");

	UI_Printf("STR_FILTER_BOOKREM_TIP");
	readline(cond[0].value, CP_COND_VALUE_SIZE);

	UI_Printf("STR_PROCESSING");
	LM_RemoveBooksWithConds(2, cond, true);
}

void CP_RescueOne(void) {
	CP_Condition cond[1];

	strcpy(cond[0].name, "STR_BOOK_NUMBER");
	cond[0].cond = '=';

	UI_Printf("STR_FILTER_BOOKRSC_TIP");
	readline(cond[0].value, CP_COND_VALUE_SIZE);

	UI_Printf("STR_PROCESSING");
	LM_RescueBooksWithConds(1, cond, true);
}

void CP_RescueAllWithExistingConds(bool AND) {
	UI_Printf("STR_PROCESSING");
	LM_RescueBooksWithConds(global_condition_cnt, global_filter_condition, AND);
}

void CP_DeleteOne(void) {
	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(false))
		return;

	CP_Condition cond[1];
	strcpy(cond[0].name, "STR_BOOK_NUMBER");
	cond[0].cond = '=';

	UI_Printf("STR_FILTER_BOOKDEL_TIP");
	readline(cond[0].value, CP_COND_VALUE_SIZE);

	UI_Printf("STR_PROCESSING");
	LM_DeleteBooksWithConds(1, cond, true);
}

void CP_ConditionGen(void) {
	char name[CP_COND_NAME_SIZE];
	UI_Printf("STR_FILTER_GEN_NAME");
	readline(name, CP_COND_NAME_SIZE);

	LM_ConditionFileGen(global_condition_cnt, name, global_filter_condition);
}

void CP_FilterSortRequestMake(char* addition) {
	memset(addition, 0, CP_EXEC_SIZE);

	int nattr = BA_GetNattr(), i, cnt = 0, input;
	char** attr = (char**)malloc((nattr + 1) * sizeof(char*));

	for (i = 0; i <= nattr; i++) {
		attr[i] = (char*)malloc(CP_NATTR_SIZE * sizeof(char));
		BA_GetAttr(i, attr[i], NULL);
	}

	strcpy(attr[nattr], "STR_FILTER_SORT_OK");
	char title[CP_FORMAT_SIZE];
	char* format = str("STR_FILTER_SORT_DEPTH");

	strcat(addition, " ORDER BY ");

	while (true) {
		char* tattr[CP_NATTR_SIZE];
		int ntattr = 0;

		for (i = 0; i <= nattr; i++) {
			if (!strcmp(attr[i], ""))
				continue;
			tattr[ntattr] = (char*)malloc(CP_NATTR_SIZE * sizeof(char));
			strcpy(tattr[ntattr], attr[i]);
			ntattr++;
		}

		if (ntattr == 1)
			break;

		sprintf(title, format, cnt + 1);
		UI_MenuMakeWithArray(title, ntattr, tattr);

		do {
			input = UI_Input();
		} while (input > ntattr || input < 1);

		if (input != ntattr) {
			// added to exec
			if (ntattr != nattr + 1)
				strcat(addition, ", ");
			strcat(addition, (tattr[input - 1]));
		}

		cnt++;
		strcpy(attr[arrIndexOf(attr, tattr[input - 1], nattr + 1)], "");
		for (i = 0; i < ntattr; i++)
			free(tattr[i]);
		if (input == ntattr)
			break;
	}
	free(format);

	// ask for ASC or DESC
	UI_MenuMake("STR_FILTER_SORT_TYPE_TIP", 2,
		"STR_FILTER_SORT_ASC",
		"STR_FILTER_SORT_DSC");

	do {
		input = UI_Input();
	} while (input > 2 || input < 1);

	strcat(addition, input == 1 ? " ASC" : " DESC");

	// release
	for (i = 0; i < nattr; i++)
		free(attr[i]);
	free(attr);
}

void CP_ModifyOne(void) {
	if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(false))
		return;

	CP_Condition cond[1];
	strcpy(cond[0].name, "STR_BOOK_NUMBER");
	cond[0].cond = '=';
	// condition make

	UI_Printf("STR_FILTER_BOOKMOD_TIP");
	readline(cond[0].value, CP_COND_VALUE_SIZE);
	// input book number

	if (getInt(PATH_CONFIG, "trygui"))
		if (UI_BookModifyWithGUI(cond[0].value))
			return;
	// ui first


	char** modifiedItems =
		(char**)malloc(CP_COND_SIZE * sizeof(char*));
	char** newValues =
		(char**)malloc(CP_COND_SIZE * sizeof(char*));

	putchar('\n');

	int nattr = BA_GetNattr(), i, cnt = 0;
	char** attr = LM_DynamicArray2Make(CP_NATTR_SIZE, nattr + 1);
	for (i = 0; i <= nattr; i++) {
		char* label = BA_GetLabel(i, 't');
		if (!strcmp(label, "RO")) {
			free(label);
			continue;
		}
		BA_GetAttr(i, attr[cnt], NULL);
		cnt++;
		free(label);
	}
	strcpy(attr[cnt - 1], "STR_MODIFY_OK");
	char title[CP_FORMAT_SIZE];
	char* format = str("STR_MODIFY_DEPTH");

	nattr = cnt - 1;
	cnt = 0;

	while (true) {
		char* tattr[CP_NATTR_SIZE];
		int ntattr = 0;
		for (i = 0; i <= nattr; i++) {
			if (!strcmp(attr[i], ""))
				continue;
			tattr[ntattr] = (char*)malloc(CP_NATTR_SIZE * sizeof(char));
			strcpy(tattr[ntattr], attr[i]);
			ntattr++;
		}
		if (ntattr == 1)
			break;

		sprintf(title, format, cnt + 1);
		UI_MenuMakeWithArray(title, ntattr, tattr);
		int input = UI_Input();
		if (input > ntattr || input < 1)
			continue;

		if (input != ntattr) {
			int index = arrIndexOf(attr, tattr[input - 1], nattr + 1);
			modifiedItems[cnt] =
				(char*)malloc(CP_COND_NAME_SIZE * sizeof(char));
			newValues[cnt] =
				(char*)malloc(CP_COND_VALUE_SIZE * sizeof(char));
			UI_Printf("STR_ADD_INPUT");
			UI_Printf("%s", tattr[input - 1]);
			UI_AttributedInput(index, attr[index], newValues[cnt]);
			strcpy(modifiedItems[cnt], (tattr[input - 1]));
			cnt++;
			strcpy(attr[index], "");
		}
		for (i = 0; i < ntattr; i++)
			free(tattr[i]);
		if (input == ntattr)
			break;
	}

	LM_FreeDynamicArray(attr, nattr + 1);

	char exec[CP_EXEC_SIZE];
	LM_UpdateExecMakeWithConds(1, cond, cnt, modifiedItems, newValues, exec);

	for (i = 0; i < cnt; i++) {
		free(modifiedItems[i]);
		free(newValues[i]);
	}
	free(modifiedItems);
	free(newValues);
	free(format);


	UI_Printf("STR_PROCESSING");
	LM_SQLExec(sql, exec, NULL, NULL, NULL);

	if (strcmp(sqlite3_errmsg(sql), "not an error")) {
		UI_MessageBox("STR_MOD_ERROR", false);
	}

}

void CP_ExportExcelNavi(char* addition, bool AND) {
	
	char exec[CP_EXEC_SIZE];
	char** data;
	int nrow, ncol;
	
	LM_ConditionExecMakeOpt("select * ", global_condition_cnt, global_filter_condition, exec, addition, -1, 0, AND);
	LM_QueryBookWithSqlExec(exec, &nrow, &ncol, &data);

	if (ncol == 0)
		return;

	NEWLINE;
	LPADDING;
	UI_Printf("STR_FILTER_INPPUT_DOCNAME");

	char docPath[CP_COND_VALUE_SIZE];
	readline(docPath, CP_COND_VALUE_SIZE);
	if (!strstr(docPath, ".txt"))
		strcat(docPath, ".txt");

	FILE* hFile = fopen(docPath, "a");
	if (!hFile)
		return;

	UI_ListMakeWithSqlite3Array("STR_FILTER_EXPORT", UI_PrintBookHeader, nrow, ncol, false, 0, data, NULL, hFile);

	fprintf(hFile, "\n");
	fclose(hFile);
	sqlite3_free_table(data);
}

void CP_FilterWithExistingConds(bool AND, int offset,
	bool flagShowDeleted) {
	UNREFERENCED_PARAMETER(offset);
	CP_FUNCTION_ADDITION;

	static int bookpage = 0;
	static char addition[CP_EXEC_SIZE] = { '\0' };
	char exec[CP_EXEC_SIZE];
	int r, c;
	int limit = getInt(PATH_CONFIG, "limit");
	char** data;

	while (true) {
		system(clearCmd());

		if (flagShowDeleted) {
			strcpy(global_filter_condition[global_condition_cnt].name, "PRESERVED_DELETED");
			strcpy(global_filter_condition[global_condition_cnt].value, "1");
			global_filter_condition[global_condition_cnt].cond = '=';
			global_condition_cnt++;
		}


		LM_ConditionExecMakeOpt("select * ", global_condition_cnt, global_filter_condition, exec, addition, limit, bookpage * limit, AND);

		if (flagShowDeleted)
			global_condition_cnt--;

		// view exec

		char* str_libman_page_tem = str("STR_LIBMAN_PAGE");
		char str_libman_page[CP_COND_SIZE];
		sprintf(str_libman_page, str_libman_page_tem, 1 + bookpage * limit, bookpage * limit + limit);
		free(str_libman_page_tem);

		// strcpy(addition, "");

		LM_QueryBookWithSqlExec(exec, &r, &c, &data);

		UI_ListMakeWithSqlite3Array(str_libman_page, UI_PrintBookHeader, r, c,
			flagShowDeleted, bookpage * limit, data, NULL, stdout);

		if (flagShowDeleted) {
			UI_MenuMake("STR_FILTER_RESULT", 8,
				"STR_FILTER_SORTING",
				"STR_LIBMAN_PREVIOUS",
				"STR_LIBMAN_NEXT",
				"STR_FILTER_EMPTY_IN_TRASHBIN",
				"STR_TRASHBIN_REMOVEONE",
				"STR_LIBMAN_RESCUEALL",
				"STR_LIBMAN_RESCUEONE",
				"STR_RETURN");
		}
		else {
			UI_MenuMake("STR_FILTER_RESULT", 8,
				"STR_FILTER_SORTING",
				"STR_LIBMAN_PREVIOUS",
				"STR_LIBMAN_NEXT",
				"STR_FILTER_DELETE_ONE",
				"STR_FILTER_DELETE",
				"STR_LIBMAN_MODIFYONE",
				"STR_FILTER_EXPORT",
				"STR_RETURN");
		}

		while (!pass) {
			pass = 1;
			if (flagShowDeleted) {
				switch (input = UI_Input()) {
				case 1: // deleted sort
					CP_FilterSortRequestMake(addition);
					break;
				case 2: // Previous page
					if (bookpage > 0)
						bookpage--;
					break;
				case 3: // Next page
					bookpage++;
					break;
				case 4: // remove all
					CP_RemoveAll(AND);
					break;
				case 5: // remove one
					CP_RemoveOne();
					break;
				case 6: // rescue all
					CP_RescueAllWithExistingConds(AND);
					break;
				case 7: // rescue one
					CP_RescueOne();
					break;
				case 8: // return
					sqlite3_free_table(data);
					// CP_AdvancedFilter(CP_TrashBin);
					return;
				default:
					pass = 0;
				}
			}
			else { // there not deleted
				switch (input = UI_Input()) {
				case 1: // sort
					CP_FilterSortRequestMake(addition);
					break;
				case 2: // Previous page
					if (bookpage > 0)
						bookpage--;
					break;
				case 3: // Next page
					bookpage++;
					break;
				case 4: // delete one
					CP_DeleteOne();
					break;
				case 5: // delete all
					CP_DeleteAll(AND);
					break;
				case 6: // modify one
					CP_ModifyOne();
					break;
				case 7: // export to excel
				{
					// TODO
					CP_ExportExcelNavi(addition, AND);
					break;
				}
				case 8: // return
					sqlite3_free_table(data);
					return;
					// CP_AdvancedFilter(CP_BookManage);
				default:
					pass = 0;
				}
			}
		}
		pass = 0;
	}
}

void CP_LoadConditionFile(void) {
	char cmd[CP_COND_VALUE_SIZE], file_name[CP_COND_VALUE_SIZE];
	strcpy(cmd, lsCmd());
	strcat(cmd, PATH_CONDSA);
	NEWLINE;
	LPADDING;
	UI_Printf("STR_FILTER_LOAD_SHOW");
	system(cmd);
	NEWLINE;
	LPADDING;
	UI_Printf("STR_FILTER_LOAD_TIP");
	readline(file_name, CP_COND_VALUE_SIZE);
	int len = strlen(file_name), i;
	for (i = len - 1; i >= 0; i--)
		if (file_name[i] == '.') {
			file_name[i] = '\0';
			break;
		}
	LM_ConditionFileLoad((file_name), global_filter_condition, &global_condition_cnt);
}

void CP_AdvancedFilter(void(*from)(void)) {
	CP_FUNCTION_ADDITION;

	while (true) {
		system(clearCmd());

		NEWLINE;
		TPADDNING;
		UI_Printf("STR_FILTER_PAGE_TIP");
		char* filter_header = str("STR_FILTER_HEADER");
		char* filter_template = str("STR_FILTER_TEMPLATE");
		char format[CP_FORMAT_SIZE];

		putchar('\n');
		LPADDING;
		LPADDING;
		LPADDING;
		UI_Printf(filter_header);
		printf("      ----------------------------------------------------\n");

		int i;
		for (i = 0; i < global_condition_cnt; i++) {
			char* pname = str((global_filter_condition[i].name));
			char* pvalue = global_filter_condition[i].value;
			char* pcond = NULL;
			bool flagLike = false;
			switch (global_filter_condition[i].cond) {
			case '=':
				pcond = str("STR_COND_EQUAL");
				break;
			case '>':
				pcond = str("STR_COND_GREATER");
				break;
			case '<':
				pcond = str("STR_COND_LESS");
				break;
			case 'N':
				pcond = str("STR_COND_NEQ");
				break;
			case 'L':
				pcond = str("STR_COND_LIKE");
				flagLike = true;
				break;
			}


			sprintf(format,	filter_template, i + 1, pname, pcond, pvalue); // index
			
			UI_Printf("   %d. ", i + 1);
			UI_Printf(format);
			free(pname);
			free(pcond);
		}

		free(filter_header);
		free(filter_template);

		UI_MenuMake("STR_LIBMAN_FILTER", 7,
			"STR_FILTER_ADD",
			"STR_FILTER_DEL",
			"STR_FILTER_IMP",
			"STR_FILTER_SAV",
			"STR_FILTER_START_AND",
			"STR_FILTER_START_OR",
			"STR_RETURN");

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // add filter
				CP_AddConditionNavi();
				break;
			case 2: // del filter
				CP_DeleteFilter();
				break;
			case 3: // imp filter
				CP_LoadConditionFile();
				break;
			case 4: // exp filter
				CP_ConditionGen();
				break;
			case 5: // filter with and
				CP_FilterWithExistingConds(true, 0, *from == *CP_TrashBin);
				break;
			case 6: // filter with or
				CP_FilterWithExistingConds(false, 0, *from == *CP_TrashBin);
				break;
			case 7:
				// from();
				return;
				// break;
			default:
				pass = 0;
			}
		}
		pass = 0;

	}
}

void CP_BookImport(void) {
	system(clearCmd());

	const int ninter = getInt(PATH_BOOK_ATTR, "ninter") + 1;
	int i;
	char** interList =
		LM_DynamicArray2Make(CP_COND_NAME_SIZE, ninter);
	char finter[CP_COND_VALUE_SIZE];
	char fcinter[CP_COND_VALUE_SIZE];

	for (i = 0; i < ninter - 1; i++) {
		char fintert[CP_COND_VALUE_SIZE];
		sprintf(fintert, "inter%dt", i + 1);

		char* interName = config(PATH_BOOK_ATTR, fintert);

		// interList[i]
		strcpy(interList[i], interName);
		free(interName);
	}
	strcpy(interList[i], "STR_RETURN");

	UI_MenuMakeWithArray("STR_INTERFACE_SELECT",
		ninter, interList);

	LM_FreeDynamicArray(interList, ninter);

	int input;
	while ((input = UI_Input()) > ninter
		|| input < 1);

	if (input == ninter)
		return;

	sprintf(finter, "inter%dv", input);
	sprintf(fcinter, "inter%dc", input);
	const int cinter = getInt(PATH_BOOK_ATTR, fcinter);

	int* order = (int*)malloc(cinter * sizeof(int));
	LM_GetOrderWithCondName(finter, order, cinter);

	char path[CP_EXEC_SIZE];

	NEWLINE;
	LPADDING;
	UI_Printf("STR_BOOK_IMPORT_TIP");
	readline(path, CP_EXEC_SIZE);
	NEWLINE;
	LPADDING;
	UI_Printf("STR_BOOK_IMPORT_LOADING");

	int nsuc = 0, nfail = 0;
	bool flagFatalError = LM_AddBookWithFile(path, cinter, order, &nsuc, &nfail);

	if (!flagFatalError) {
		char* msgFatalError = str("STR_BOOK_IMPORT_FATAL_ERROR");
		UI_MessageBox(msgFatalError, false);
		free(msgFatalError);
	}
	else {
		char* msgCompleteFormat = str("STR_BOOK_IMPORT_COMPLETE");
		char msgComplete[CP_FORMAT_SIZE];
		sprintf(msgComplete, msgCompleteFormat, nsuc, nfail);
		UI_MessageBox(msgComplete, false);
		free(msgCompleteFormat);
	}

	free(order);
}

void CP_TrashBin(void) {
	CP_FUNCTION_ADDITION;

	while (true) {
		if (getInt(PATH_CONFIG, "safe_mode") && !UI_MastercodeCheck(true))
			return;

		static int bookpage = 0;
		int limit = getInt(PATH_CONFIG, "limit");

		system(clearCmd());

		int r, c;
		char** data;
		char exec[CP_EXEC_SIZE];

		char* str_libman_page_tem = str("STR_LIBMAN_PAGE");
		char str_libman_page[CP_COND_VALUE_SIZE];
		sprintf(str_libman_page, str_libman_page_tem,
			1 + bookpage * limit, bookpage * limit + limit);
		free(str_libman_page_tem);

		CP_Condition deletedCond[1];
		strcpy(deletedCond[0].name, "PRESERVED_DELETED");
		strcpy(deletedCond[0].value, "1");
		deletedCond[0].cond = '=';

		LM_SelectConditionExecMake(1, deletedCond, exec, limit, bookpage * limit, true);
		LM_QueryBookWithSqlExec(exec, &r, &c, &data);
		
		UI_ListMakeWithSqlite3Array(str_libman_page, UI_PrintBookHeader,
			r, c, true, bookpage * limit, data, NULL, stdout);

		UI_MenuMake("STR_LIBMAN_RECOVER", 7,
			"STR_LIBMAN_FILTER",
			"STR_LIBMAN_PREVIOUS",
			"STR_LIBMAN_NEXT",
			"STR_TRASHBIN_REMOVEALL",
			"STR_TRASHBIN_REMOVEONE",
			"STR_LIBMAN_RESCUEONE",
			"STR_RETURN");

		sqlite3_free_table(data);

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // filter
				CP_AdvancedFilter(CP_TrashBin);
				break;
			case 2: // Previous page
				if (bookpage > 0)
					bookpage--;
				break;
			case 3: // Next page
				bookpage++;
				break;
			case 4: // Remove all
			{
				if (UI_Prompt("STR_TRASHBIN_RMALL_CONFIRM", false)) {
					global_condition_cnt = 0;
					CP_RemoveAll(true);
				}
				break;
			}
			case 5: // remove one
				CP_RemoveOne();
				break;
			case 6: // rescue one
				CP_RescueOne();
				break;
			case 7: // return
				return;
			default:
				pass = 0;
			}
		}
		pass = 0;

	}
}

bool _CP_SearchJudge(char* str) {
	// returns if str contains all keyword.
	for (int i = 0; i < global_nkeyword; i++)
		if (!strstr(str, global_keywords[i]))
			return false;
	return true;
}

void CP_Search(void) {
	LPADDING;
	char rawKeywords[CP_EXEC_SIZE];

	CP_Condition backupCond[CP_COND_NAME_SIZE];
	int backupCondCnt = 0;

	UI_Printf("STR_SEARCH_TIP");
	UI_Printf(": ");
	readline(rawKeywords, CP_EXEC_SIZE);
	if (!strcmp(rawKeywords, "miku"))
		UI_MessageBox("miku! \\^o^/~!", true);
	
	if (!global_keywords)
		global_keywords = LM_DynamicArray2Make(CP_COND_VALUE_SIZE, CP_COND_NAME_SIZE);
	
	split(rawKeywords, ' ', global_keywords, &global_nkeyword);

	// backup
	backupCondCnt = global_condition_cnt;
	for (int i = 0; i < global_condition_cnt; i++)
		CP_ConditionCpy(backupCond + i, global_filter_condition + i);

	global_condition_cnt = global_nkeyword;
	for (int i = 0; i < global_condition_cnt; i++)
		CP_ConditionMake(global_filter_condition + i, "STR_BOOK_NAME", 'L', global_keywords[i]);

	CP_FilterWithExistingConds(true, 0, false);

	// recover
	global_condition_cnt = backupCondCnt;
	for (int i = 0; i < global_condition_cnt; i++)
		CP_ConditionCpy(global_filter_condition + i, backupCond + i);
}

void CP_BookManage(void) {
	CP_FUNCTION_ADDITION;

	while (true) {
		static int bookpage = 0;
		int limit = getInt(PATH_CONFIG, "limit");

		system(clearCmd());

		int r, c;
		char** data;

		char* str_libman_page_tem = str("STR_LIBMAN_PAGE");
		char str_libman_page[CP_COND_VALUE_SIZE];
		sprintf(str_libman_page, str_libman_page_tem,
			1 + bookpage * limit, bookpage * limit + limit);
		free(str_libman_page_tem);

		LM_QueryBookAll(&r, &c, &data, limit, bookpage * limit);

		UI_ListMakeWithSqlite3Array(str_libman_page, UI_PrintBookHeader, r, c,
			false, bookpage * limit, data, NULL, stdout);

		UI_MenuMake("STR_BOOK_LOOKUP", 8,
			"STR_LIBMAN_FILTER",
			"STR_LIBMAN_PREVIOUS",
			"STR_LIBMAN_NEXT",
			"STR_LIBMAN_SEARCH",
			"STR_LIBMAN_MODIFY",
			"STR_LIBMAN_RECOVER",
			"STR_LIBMAN_IMPORT",
			"STR_RETURN");

		sqlite3_free_table(data);

		while (!pass) {
			pass = 1;
			switch (input = UI_Input()) {
			case 1: // Book filter
				CP_AdvancedFilter(CP_BookManage);
				break;
			case 2: // Previous page
				if (bookpage > 0)
					bookpage--;
				break;
			case 3: // Next page
				bookpage++;
				break;
			case 4: // search
				CP_Search();
				break;
			case 5: // Book modify
				CP_ModifyOne();
				break;
			case 6: // Trash bin
				CP_TrashBin();
				break;
			case 7: // import
				CP_BookImport();
				break;
			case 8: // return
				return;
			default:
				pass = 0;
			}
		}
		pass = 0;
	}
}
