/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "operation.h"

namespace csp
{
	class OpLua : public Operation
	{
	public:
		OpLua();
		virtual ~OpLua();

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual void Terminate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual int PushResults( lua::LuaStack& luaStack );

		void UnrefSelf( lua::LuaStack& luaStack );
		bool CallLua( lua::LuaStack& stack, const char* functionName, int numArgs, int numRets );

		lua::LuaRef_t m_self;
	};

	void InitializeOpLua( lua::LuaState& state );
	void ShutdownOpLua( lua::LuaState& state );
}