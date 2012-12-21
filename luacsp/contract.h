#pragma once
#include "csp.h"

namespace csp
{
	void InitializeContracts( lua::LuaState& state );
	void ShutdownContracts( lua::LuaState& state );
}
