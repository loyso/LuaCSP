/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "luacpp.h"

#include "luastack.h"

namespace lua
{
	class LuaState
	{
	public:
		static LuaState NewState();
		static void CloseState( LuaState & luaState );

		void Close();

		LuaState();
		explicit LuaState(lua_State* luaState);
		lua_State* InternalState() const;

		Return::Enum LoadFromMemory( const void* data, size_t size, const char* chunkname );
		Return::Enum Call( int numArgs, int numResults );
		Return::Enum Resume( int numArgs, LuaState * pStateFrom );

		void CheckStack() const;		

		int GetTop() const;
		LuaStackValue GetTopValue() const;

		LuaStack& GetStack();

		// Per-thread user data requires the Lua interpreter to be compiled with LUAI_EXTRASPACE=sizeof(void*)
		static void* GetUserData( lua_State* luaState );
		static void SetUserData( lua_State* luaState, void* userData );

		Return::Enum PrintError( Return::Enum result );

		int Yield( int numArgs );

		Return::Enum Status() const;

		void ReportRefLeaks() const;
	private:
		LuaStack m_stack;
	};
}
