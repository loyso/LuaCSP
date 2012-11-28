#pragma once

#include <assert.h>
#include <stddef.h>

#include <luacpp/luacpp.h>

namespace csp
{
    class Operation;

    class Process
    {
	public:
		Process( lua::LuaState & luaThread, Process * parentProcess );
		~Process();

		lua::LuaState & LuaThread();

    private:
        Process* m_parentProcess;
		lua::LuaState m_luaThread;
        Operation* m_operation;
    };
}