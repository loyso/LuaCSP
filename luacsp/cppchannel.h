/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "csp.h"

#include <luacpp/luastackvalue.h>

#include "channel.h"

namespace csp
{
	class OpCppChannelOut : public OpChannel, ChannelAttachmentOut_i
	{
	protected:
		OpCppChannelOut();
		virtual ~OpCppChannelOut();

		virtual bool Init( lua::LuaStack& args, InitError& initError );

	private:
		bool IsOutputAttached();

		virtual WorkResult::Enum Evaluate( Host& host );

		void MemorizeOutputArguments( lua::LuaStack &stack );

		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual int PushResults( lua::LuaStack& luaStack );
		virtual void Terminate( Host& host );

		virtual Process& ProcessToEvaluate();
		virtual void Communicate( Host& host, Process& inputProcess );
		virtual void CloseChannel( Host& host, Channel& channel );

		virtual WorkResult::Enum Update( CspTime_t dt ) = 0;
		virtual bool IsOutputReady() const = 0;
		virtual int PushOutputArguments( lua::LuaStack& luaStack ) = 0;
	};

	void InitializeCppChannels( lua::LuaState& state );
	void ShutdownCppChannels( lua::LuaState& state );
}
