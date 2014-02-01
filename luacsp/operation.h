/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "csp.h"
#include "process.h"

namespace lua
{
	class LuaStackValue;
}

namespace csp
{
	class Process;
	class Channel;
	class Operation;
}

namespace csp
{
    class Operation
    {
	public:
		Operation();
		virtual ~Operation();

		int DoInit( lua_State* luaState );
		void DoTerminate( Host& host );

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt ) = 0;
		
		virtual int PushResults( lua::LuaStack& luaStack );

		bool IsFinished() const;
		void SetFinished( bool finished );

		virtual void DebugCheck( Host& host ) const;

	protected:
		Process& ThisProcess() const;

		struct InitError
		{
			bool ArgError( int arg, const char* message );
			bool Error( const char* message );

			int errorArg;
			const char* errorMessage;
		};

    private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual void Terminate( Host& host );

		Process* m_pProcess;
		bool m_finished;
    };

	class OpSleep : public Operation
	{
	public:
		OpSleep();

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );

		csp::CspTime_t m_seconds;
	};

	void RegisterStandardOperations( lua::LuaState& state, lua::LuaStackValue& value );
	void UnregisterStandardOperations( lua::LuaState& state, lua::LuaStackValue& value );
}
