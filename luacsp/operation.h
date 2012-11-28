#pragma once

#include <stddef.h>
#include <assert.h>

#include "csp.h"

namespace lua
{
	class LuaStackValue;
}

namespace csp
{
    class Process;

    class Operation
    {
	public:
		Operation();
		~Operation();

		void SetProcess( Process & process );

		virtual WorkResult::Enum Work( time_t dt, int & outNumRets) = 0;

    private:
		Process* m_pProcess;
    };

	class OpSleep : public Operation
	{
	public:
		OpSleep( time_t seconds );

		virtual WorkResult::Enum Work( time_t dt, int & outNumRets );

	private:
		float m_seconds;
	};

	struct OperationDescription
	{
		int (*function)( lua_State* L );
		const char* name;
	};


	void RegisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] );
	void UnregisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] );

	void RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
	void UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
}