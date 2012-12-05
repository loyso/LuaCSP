#pragma once

#include "csp.h"
#include "operation.h"

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

	class OpChannel : public Operation
	{
	public:
		OpChannel();
		virtual ~OpChannel();

		virtual bool Init( lua::LuaStack& args );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	protected:
		void UnrefChannel( lua::LuaStack const& stack );

	private:
		lua::LuaRef_t m_channelRefKey;
		Channel* m_pChannel;
	};

	class OpChannelIn : public OpChannel
	{
	public:
		OpChannelIn();
		virtual ~OpChannelIn();

		virtual WorkResult::Enum Evaluate( Host& host );
	};

	class OpChannelOut : public OpChannel
	{
	public:
		OpChannelOut();
		virtual ~OpChannelOut();

		virtual WorkResult::Enum Evaluate( Host& host );
	};

	void PushChannel( lua_State* luaState, Channel& channel );
	bool IsChannelArg( lua::LuaStackValue const& value );
	Channel* GetChannelArg( lua::LuaStackValue const& value );

	void InitializeChannels( lua::LuaState& state );
	void ShutdownChannels( lua::LuaState& state );
}
