
/*
	Name:			NewStd.h
	Description:	bool support and other features.
*/


#pragma once

// bool
#include <stdbool.h>

// disable warnings about unreferenced params.
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (P)
#endif

// wchar_t deprecated due to linux compatible issue.

/*
	#define char char_t
	// make it more convenient to use wide char.
*/

