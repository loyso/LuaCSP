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
	struct ChannelAttachmentIn_i;
	struct ChannelAttachmentOut_i;
	struct ChannelArgument;

	class GcObject;
	class Channel;
}

namespace csp
{
	class GcObject
	{
	public:
		GcObject();
		virtual ~GcObject();
	};

	struct ChannelAttachment_i
	{
		virtual Process& ProcessToEvaluate() = 0;
	};

	struct ChannelAttachmentIn_i : ChannelAttachment_i
	{
		virtual void MoveChannelArguments( ChannelArgument* arguments, int numArguments ) = 0;
	};

	struct ChannelAttachmentOut_i : ChannelAttachment_i
	{
		virtual void MoveChannelArguments() = 0;
	};


	class Channel : public GcObject
	{
	public:
		Channel();
		virtual ~Channel();

		void SetAttachmentIn( ChannelAttachmentIn_i* pAttachment );
		void SetAttachmentOut( ChannelAttachmentOut_i* pAttachment );
	
		bool InAttached() const;
		bool OutAttached() const;

		ChannelAttachmentIn_i& InAttachment() const;
		ChannelAttachmentOut_i& OutAttachment() const;

	private:
		ChannelAttachmentIn_i* m_pAttachmentIn;
		ChannelAttachmentOut_i* m_pAttachmentOut;
	};

	struct ChannelArgument
	{
		lua::LuaRef_t refKey;
	};

	class OpChannel : public Operation
	{
	public:
		OpChannel();
		virtual ~OpChannel();

		virtual WorkResult::Enum Work( Host& host, time_t dt );

	protected:
		bool InitChannel( lua::LuaStack& args, InitError& initError );
		void UnrefChannel( lua::LuaStack const& stack );

		void InitArguments( lua::LuaStack& args, InitError& initError );
		void UnrefArguments( lua::LuaStack const& stack );

		bool HasChannel();
		Channel& ThisChannel();

		void MoveChannelArguments( ChannelArgument* arguments, int numArguments );
		void ArgumentsMoved();
		
		ChannelArgument* Arguments() const;
		int NumArguments() const;
		bool HaveArgumentsMoved() const;

	private:
		lua::LuaRef_t m_channelRefKey;
		Channel* m_pChannel;

		ChannelArgument* m_arguments;
		int m_numArguments;
		bool m_argumentsMoved;
	};

	class OpChannelIn : public OpChannel, ChannelAttachmentIn_i
	{
	public:
		OpChannelIn();
		virtual ~OpChannelIn();

	private:
		virtual bool Init( lua::LuaStack & args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual int PushResults( lua::LuaStack & luaStack );
		
		virtual Process& ProcessToEvaluate();
		virtual void MoveChannelArguments( ChannelArgument* arguments, int numArguments );
	};

	class OpChannelOut : public OpChannel, ChannelAttachmentOut_i
	{
	public:
		OpChannelOut();
		virtual ~OpChannelOut();

	private:
		virtual bool Init( lua::LuaStack & args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );

		virtual Process& ProcessToEvaluate();
		virtual void MoveChannelArguments();
	};

	void PushChannel( lua_State* luaState, Channel& channel );
	bool IsChannelArg( lua::LuaStackValue const& value );
	Channel* GetChannelArg( lua::LuaStackValue const& value );

	void InitializeChannels( lua::LuaState& state );
	void ShutdownChannels( lua::LuaState& state );
}
