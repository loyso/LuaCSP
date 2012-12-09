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

void* lua::LuaDefaultAlloc (void* ud, void* ptr, size_t osize, size_t nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

lua::LuaReader::LuaReader(const void* data, size_t size)
	: m_Data((const uint8_t*)data)
	, m_Size(size)
{
}

const char* lua::LuaReader::Read(lua_State *L, void *data, size_t *size)
{
	(void)L;
	LuaReader* pThis = (LuaReader*)data;

	const char* result = NULL;
	*size = 0;

	if(pThis->m_Size > 0)
	{
		*size = pThis->m_Size;
		result = (const char*)pThis->m_Data;

		pThis->m_Data += pThis->m_Size;
		pThis->m_Size -= *size;
	}

	return result;
}


