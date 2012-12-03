#pragma once

#include <luacpp/luacpp.h>

#include "csp.h"
#include "process.h"

#include <stack>

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

        WorkResult::Enum Main();
		WorkResult::Enum Work( time_t dt );

        lua::LuaState& LuaState();

		void PushEvalStep( Process& process );
		Process& PopEvalStep();

    private:
		void Evaluate();

        lua::LuaState m_luaState;
		Process m_mainProcess;

		std::stack< Process* > m_evalSteps;
    };
}
