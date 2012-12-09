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

		int Error( const char* format, ... );
		int ArgError( int arg, const char* format, ... );
		Return::Enum PrintError( Return::Enum result );

		int Yield( int numArgs );

		Return::Enum Status() const;

		void ReportRefLeaks() const;
	private:
		LuaStack m_stack;
	};
}
