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

	class LuaState;
	class LuaStackValue;

	class LuaStackValue
	{
	public:
		LuaStackValue(LuaState const & luaState, int index);

		bool IsNil() const;
		bool IsFunction() const;
		bool IsCFunction() const;

	private:
		lua_State * m_state;
		int m_index;
	};

    class LuaState
    {
    public:
        static LuaState NewState();
        
        void Close();

        LuaState(lua_State& state);
        lua_State* InternalState() const;

        Return::Value LoadFromMemory(const void* data, size_t size, const char* chunkname);
		Return::Value Call(int numArgs, int numResults);

		LuaState NewThread();

		void CheckStack() const;		
		void Pop(int numValues);

		int GetTop() const;
		LuaStackValue GetTopValue() const;

		LuaStackValue GetGlobal(const char * var) const;

        // Per-thread user data requires the Lua interpreter to be compiled with LUAI_EXTRASPACE=sizeof(void*)
        void* GetUserData() const;
        void SetUserData(void* userData);

    private:
        lua_State* m_state;
    };
}