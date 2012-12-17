#pragma once
#include "csp.h"

#include <luacpp/luastackvalue.h>

#include "channel.h"

namespace csp
{
}

namespace csp
{
	class OpCppChannelOut : public OpChannel, ChannelAttachmentOut_i
	{
	protected:
		OpCppChannelOut();
		virtual ~OpCppChannelOut();

		virtual bool Init( lua::LuaStack& args, InitError& initError );

	private:
		bool AttachOutput();
		bool IsOutputAttached();
		bool DetachOutput();

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );
		virtual int PushResults( lua::LuaStack& luaStack );
		virtual void Terminate( Host& host );

		virtual Process& ProcessToEvaluate();
		virtual void MoveChannelArguments();

		virtual WorkResult::Enum Update( time_t dt ) = 0;
		virtual bool IsOutputReady() const = 0;
		virtual ChannelArgument* CreateArguments( lua::LuaStack& stack, int& numArguments ) = 0;
	};

	void InitializeCppChannels( lua::LuaState& state );
	void ShutdownCppChannels( lua::LuaState& state );
}
