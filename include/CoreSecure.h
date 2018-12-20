
/*
	Name:			CoreSecure.h
	Description:	Security question and password management.
*/

#pragma once

#include <string.h>
#include "NewStd.h"

// check password.
bool SM_MasterCodeCheck(char* uCode);

// update password.
bool SM_MasterCodePush(char* uCode);

// check security question.
bool SM_SQAnswerCheck(char* uAnswer);

// update security question.
bool SM_SQQuestionPush(char* uQuestion);
bool SM_SQAnswerPush(char* uAnswer);

// read security question.
void SM_SQQuestionGet(char* buffer);

