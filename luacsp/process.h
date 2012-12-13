#pragma once

#include <luacpp/luastate.h>

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

		void Work( Host& host, time_t dt );

		WorkResult::Enum StartEvaluation( Host& host, int numArgs );
		WorkResult::Enum Evaluate( Host& host, int numArgs );

		void DoTerminate( Host& host );
		void Terminate( Host& host );

		void SwitchCurrentOperation( Operation* pOperation );
		bool IsRunning() const;

    private:
		Operation& CurrentOperation();
		WorkResult::Enum Resume( int numArgs );
		void DeleteOperation( Host& host );

		lua::LuaState m_luaThread;
		Process* m_parentProcess;
        Operation* m_operation;
    };
}