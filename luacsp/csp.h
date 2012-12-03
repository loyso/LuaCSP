#pragma once

struct lua_State;

namespace lua
{
	class LuaState;
	class LuaStackValue;
}

namespace csp
{
    class Host;

	typedef float time_t;

	namespace WorkResult
	{
		enum Enum
		{
			  FINISH = 0
			, YIELD
		};
	}

	struct FunctionRegistration
	{
		const char* name;
		int (*function)( lua_State* L );
	};

	void RegisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration registrations[] );
	void UnregisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration registrations[] );

    Host& Initialize();
    void Shutdown(Host& host);
}