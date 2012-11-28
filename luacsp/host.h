#pragma once

#include <luacpp/luacpp.h>

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

        lua::Return::Value Main();

        lua::LuaState& LuaState();

    private:
        lua::LuaState m_luaState;
		Process * m_mainProcess;
    };
}
