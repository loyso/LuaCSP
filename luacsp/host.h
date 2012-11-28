#pragma once

struct lua_State;

namespace lua
{
    class LuaState;
}

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

        void Main();

        lua::LuaState LuaState();

    private:
        lua_State* m_InternalState;
    };
}
