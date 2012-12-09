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
		WorkResult::Enum Work( time_t dt );

        lua::LuaState& LuaState();
		
		time_t Time() const;
		unsigned int Tick() const;

		void PushEvalStep( Process& process );
		Process& PopEvalStep();
		Process* GetTopProcess() const;
		bool IsEvalsStackEmpty() const;

    private:
		void Evaluate();

        lua::LuaState m_luaState;
		Process m_mainProcess;

		Process** m_evalStepsStack;
		int m_evalStepsStackTop;

		unsigned int m_tick;
		time_t m_time;
    };
}
