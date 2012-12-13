#include "luacpp.h"

#include <stdarg.h>
#include <stdio.h>

int lua::Print(const char* fmt, ...)
{
	va_list ap;
	va_start( ap, fmt );
	int retval = vprintf( fmt, ap );
	va_end( ap );
	return retval;
}

void* lua::LuaDefaultAlloc( void*, void* ptr, size_t, size_t nsize ) 
{
	if( nsize == 0 ) 
	{
		free( ptr );
		return NULL;
	}
	else
		return realloc( ptr, nsize );
}

lua::LuaReader::LuaReader(const void* data, size_t size)
	: m_pData((const uint8_t*)data)
	, m_size(size)
{
}

const char* lua::LuaReader::Read(lua_State*, void* data, size_t* size)
{
	LuaReader* pThis = (LuaReader*)data;

	const char* result = NULL;
	*size = 0;

	if(pThis->m_size > 0)
	{
		*size = pThis->m_size;
		result = (const char*)pThis->m_pData;

		pThis->m_pData += pThis->m_size;
		pThis->m_size -= *size;
	}

	return result;
}
