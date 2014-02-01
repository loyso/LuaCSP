/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include <stdint.h>

struct lua_State;

namespace lua
{
	class LuaStackValue;
}

namespace lua
{
	typedef double LuaNumber_t; // must match lua_Number in luaconf.h

	typedef int LuaRef_t;
	const LuaRef_t LUA_NO_REF = -2; // matches LUA_NOREF
	const int LUA_MULT_RET = -1; // matches LUA_MULTRET
	typedef int (*CFunction_t)( lua_State*);

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
	void PrintStackValue( LuaStackValue const& value );
	void PrintStackArray( LuaStackValue const& value );
	void PrintStackTable( LuaStackValue const& value );
	
	void* LuaDefaultAlloc( void* ud, void* ptr, size_t osize, size_t nsize );

	class LuaState;
	class LuaStack;
	class LuaStackValue;

	class LuaReader
	{
	public:
		LuaReader( const void* data, size_t size );
		static const char* Read( lua_State* luaState, void* data, size_t* size );

	private:
		const uint8_t* m_pData;
		size_t m_size;
	};
}