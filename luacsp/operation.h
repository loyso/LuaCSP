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

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt ) = 0;
		
		virtual int PushResults( lua::LuaStack & luaStack );

		bool IsFinished() const;
		void SetFinished( bool finished );

	protected:
		Process& ThisProcess() const;

    private:
		virtual bool Init( lua::LuaStack & args );

		Process* m_pProcess;
		bool m_finished;
    };

	class OpSleep : public Operation
	{
	public:
		OpSleep();

		// virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	private:
		virtual bool Init( lua::LuaStack & args );
		float m_seconds;
	};

	class OpPar : public Operation
	{
	public:
		OpPar();
		virtual ~OpPar();

		virtual bool Init( lua::LuaStack& args );

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	private:
		bool CheckFinished();

		struct ParClosure
		{
			Process process;
			int refKey;
		};
		ParClosure * m_closures;
		int m_numClosures;
		int m_closureToRun;
	};


	void RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
	void UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
}