#pragma once

#include "csp.h"

struct lua_State;

namespace lua
{
	class LuaStack;
}

namespace csp
{
	class GcObject
	{
	public:
		GcObject();
		virtual ~GcObject();
	};

	class Channel : public GcObject
	{
	};

	void PushChannel( lua_State* luaState, Channel& channel );

	void InitializeChannels( lua::LuaState& state );
	void ShutdownChannels( lua::LuaState& state );
}
