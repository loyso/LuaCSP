#include "luacpp.h"

#include <stdarg.h>
#include <stdio.h>

int lua::Print(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int retval = vprintf(fmt, ap);
	va_end(ap);
	return retval;
}

