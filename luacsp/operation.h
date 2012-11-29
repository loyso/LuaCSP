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
		virtual ~Operation();

		int Initialize( lua_State* luaState );

		virtual WorkResult::Enum Work( time_t dt ) = 0;
		virtual int PushResults( lua::LuaStack & luaStack );

	protected:
		Process& GetProcess() const;

    private:
		virtual bool Init( lua::LuaStack & args );

		Process* m_pProcess;
    };

	class OpSleep : public Operation
	{
	public:
		OpSleep();

		virtual WorkResult::Enum Work( time_t dt );

	private:
		virtual bool Init( lua::LuaStack & args );
		float m_seconds;
	};

	struct OperationDescription
	{
		int (*function)( lua_State* L );
		const char* name;
	};

	class OpPar : public Operation
	{
	public:
		OpPar();
		virtual ~OpPar();

		virtual bool Init( lua::LuaStack& args );

		virtual WorkResult::Enum Work( time_t dt );

	private:
		WorkResult::Enum RunNextClosure();

		struct ParClosure
		{
			Process process;
			int refKey;
		};
		ParClosure * m_closures;
		int m_numClosures;
		int m_closureToRun;
	};


	void RegisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] );
	void UnregisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] );

	void RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
	void UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
}