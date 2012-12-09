#pragma once

#include <stdint.h>

struct lua_State;

namespace lua
{
	typedef float LuaNumber_t; // must be correspondent to lua_Number in luaconf.h

	typedef int LuaRef_t;
	const LuaRef_t LUA_NO_REF = -2; // LUA_NOREF

    namespace Return
    {
        enum Enum
        {
              OK = 0
            , YIELD = 1
            , ERRRUN = 2
            , ERRSYNTAX = 3
            , ERRMEM = 4
            , ERRGCMM = 5
            , ERRERR = 6
        };
    };

	int Print(const char* fmt, ...);
	void* LuaDefaultAlloc( void* ud, void* ptr, size_t osize, size_t nsize );

	class LuaState;
	class LuaStack;
	class LuaStackValue;

	class LuaReader
	{
	public:
		LuaReader( const void* data, size_t size );
		static const char* Read( lua_State *L, void *data, size_t *size );

	private:
		const uint8_t* m_Data;
		size_t m_Size;
	};
}