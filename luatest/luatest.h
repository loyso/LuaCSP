#pragma once

namespace lua
{
	class LuaState;
}

namespace lua
{
	void InitTests( LuaState& luaState );
	void ShutdownTests( LuaState& luaState );
}