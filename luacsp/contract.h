/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "csp.h"

namespace csp
{
	void InitializeContracts( lua::LuaState& state );
	void ShutdownContracts( lua::LuaState& state );
}
