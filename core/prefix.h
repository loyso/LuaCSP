#pragma once

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

#define LUAI_EXTRASPACE sizeof(void*)

#ifdef NDEBUG
#pragma warning( disable : 4702 ) // unreachable code
#endif

#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#	ifndef DBG_NEW
#		define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#		define CORE_NEW DBG_NEW
#	else
#		define CORE_NEW new
#	endif
#else  // NDEBUG
#	define CORE_NEW new
#endif
