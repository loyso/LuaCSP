#pragma once

#include <assert.h>
#include <stddef.h>

#include <luacpp/luacpp.h>

#include "csp.h"

struct lua_State;

namespace csp
{
    class Operation;

    class Process
    {
	public:
		Process();
		~Process();

		lua::LuaState & LuaThread();
		void SetLuaThread( const lua::LuaState& luaThread );
		void SetParentProcess( Process& parentProcess );

		static Process* GetProcess( lua_State* luaState );
		static void SetProcess( lua_State* luaState, Process* process );

		int Operate( Operation& operation );

		WorkResult::Enum Resume( int numArgs );
		WorkResult::Enum Work( time_t dt );

		bool IsRunning() const;

    private:
		void SwitchCurrentOperation( Operation* pOperation );
		Operation& CurrentOperation();

		lua::LuaState m_luaThread;
		Process* m_parentProcess;
        Operation* m_operation;
    };
}