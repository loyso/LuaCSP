#pragma once

#include <assert.h>
#include <stddef.h>

#include <luacpp/luacpp.h>

struct lua_State;

namespace csp
{
    class Operation;

    class Process
    {
	public:
		Process( lua::LuaState & luaThread, Process * parentProcess );
		~Process();

		lua::LuaState & LuaThread();

		static Process* GetProcess( lua_State* luaState );
		static void SetProcess( lua_State* luaState, Process* process );

		int Operate( Operation& operation );

    private:
		void SwitchCurrentOperation( Operation* pOperation );
		Operation& CurrentOperation();

        Process* m_parentProcess;
		lua::LuaState m_luaThread;
        Operation* m_operation;
    };
}