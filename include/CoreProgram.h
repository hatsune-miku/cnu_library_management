
/*
	Name:			CoreProgram.h
	Description:	Core logics. 
*/

#pragma once

#include "NewStd.h"
#include "CoreUI.h"

#define CP_COND_SIZE 64
#define CP_TFORMAT_SIZE 4
#define CP_NATTR_COUNT 64
#define CP_NATTR_SIZE 64
#define CP_FORMAT_SIZE 64
#define CP_COND_NAME_SIZE 64
#define CP_COND_VALUE_SIZE 64
#define CP_EXEC_SIZE 1024

#define LIC_PATH_SQLITE \
"licenses\\LICENSE_SQLITE.txt"
#define LIC_PATH_LIBCURL \
"licenses\\LICENSE_LIBCURL.txt"
#define LIC_PATH_OPENSSL \
"licenses\\LICENSE_OPENSSL.txt"
#define LIC_PATH_APACHEV2 \
"licenses\\LICENSE_APACHEV2.txt"

#define LPADDING printf("  ");
#define TPADDNING printf("   ");
#define NEWLINE putchar('\n');

// a condition includes a name, an operator(cond) and a value.
typedef struct {
	char name[CP_COND_NAME_SIZE];
	char cond;
	char value[CP_COND_VALUE_SIZE];
} CP_Condition;


// (view) - book management.
void CP_BookManage( void );

// (view) - a command line for custom sqlite3 exec.
void CP_AnySqlCmd( void );

// (view) - show open sourse licenses.
void CP_OpenSourceLic( void );

// (method) - condition make.
void CP_ConditionMake(
	CP_Condition* target,
	const char* name,
	const char op,
	const char* value);

// (view) - sell mode.
void CP_BuyMode( void );
void CP_BuyModeCheck( void );

// (navigation) - add a condition to advanced filter.
void CP_AddCondition(CP_Condition cond);

// (method) - add a condition to advanced filter.
void CP_ConditionCpy(CP_Condition* dst, CP_Condition* src);

// (view) - main view.
void CP_Main( void );

// (view) - cloud service.
void CP_CloudService( void );

// (view) - settings.
void CP_Settings( void );

// (view) - books with deleted tag.
void CP_TrashBin( void );

// (navigation) - switch language file.
void CP_SwitchLanguage( void );

// (view) - advanced filter.
void CP_AdvancedFilter(void(*from)( void ));

// (method) - filter.
void CP_FilterWithExistingConds(
	bool AND, 
	int offset,
	bool flagShowDeleted);

// (view) - borrow mode.
void CP_BorrowMode( void );

// (view) - search.
void CP_Search( void );

// (navigation) - delete one.
void CP_DeleteOne( void );

// (navigation) - export to excel.
void CP_ExportExcelNavi(char* addition, bool AND);

// (navigation) - add one.
void CP_AddBook( void );

// (navigation) - recover one.
void CP_RescueOne( void );

// (navigation) - reset database.
void CP_ResetDatabase( void );

// (navigation) - mass add.
void CP_BookImport( void );

// (method) - mass recover.
void CP_RescueAllWithExistingConds(bool AND);

// (method) - mass delete.
void CP_DeleteAll(bool AND);

// (navigation) - delete a condition.
void CP_DeleteFilter( void );

// (navigation) - import condition file.
void CP_LoadConditionFile( void );


