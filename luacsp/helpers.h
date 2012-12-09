#pragma once

#include "csp.h"

namespace csp
{
	void RegisterStandardHelpers( lua::LuaState & state, lua::LuaStackValue & value );
	void UnregisterStandardHelpers( lua::LuaState & state, lua::LuaStackValue & value );
}