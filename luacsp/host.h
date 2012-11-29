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

        WorkResult::Enum Main();
		WorkResult::Enum Work( time_t dt );

        lua::LuaState& LuaState();

    private:
        lua::LuaState m_luaState;
		Process m_mainProcess;
    };
}
