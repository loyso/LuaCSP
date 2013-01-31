/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include <luacpp/luacpp.h>

#include "csp.h"
#include "process.h"

namespace csp
{
    class Host
    {
    public:
        static Host& Create();

        Host(const lua::LuaState& luaState);
        ~Host();

        void Initialize();
        void Shutdown();

		static Host& GetHost( lua_State* luaState );

        WorkResult::Enum Main();
		void TerminateMain();
		WorkResult::Enum Work( CspTime_t dt );

        lua::LuaState& LuaState();
		
		CspTime_t Time() const;
		unsigned int Tick() const;

		void PushEvalStep( Process& process );
		Process& PopEvalStep();
		
		Process* GetTopProcess() const;
		bool IsEvalsStackEmpty() const;
		void RemoveProcessFromStack( const Process& process );

		bool IsProcessOnStack( const Process& process ) const;

    private:
		void Evaluate();

        lua::LuaState m_luaState;
		Process m_mainProcess;

		Process** m_evalStepsStack;
		int m_evalStepsStackTop;

		unsigned int m_tick;
		CspTime_t m_time;
    };
}
