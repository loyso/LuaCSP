#pragma once

struct lua_State;

namespace lua
{
    namespace Return
    {
        enum Value
        {
              OK = 0
            , YIELD = 1
            , ERRRUN = 2
            , ERRSYNTAX = 3
            , ERRMEM = 4
            , ERRGCMM = 5
            , ERRERR = 6
        };
    };

    class LuaState
    {
    public:
        static LuaState NewState();
        
        void Close();

        LuaState(lua_State& state);
        lua_State* InternalState() const;

        Return::Value LoadFromMemory(const void* data, size_t size, const char* chunkname);

        // Per-thread user data requires the Lua interpreter to be compiled with LUAI_EXTRASPACE=sizeof(void*)
        void* GetUserData();
        void SetUserData(void* userData);

    private:
        lua_State* m_State;
    };
}