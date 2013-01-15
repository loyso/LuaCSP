/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
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
	struct ChannelArgument
	{
		lua::LuaRef_t refKey;
	};

	const int CSP_NO_ARGS = -1;

	class Channel : public GcObject
	{
	public:
		Channel();
		virtual ~Channel();

		void SetAttachmentIn( ChannelAttachmentIn_i& attachment );
		void SetAttachmentOut( ChannelAttachmentOut_i& attachment );
	
		void ResetAttachmentIn( const ChannelAttachmentIn_i& attachment );
		void ResetAttachmentOut( const ChannelAttachmentOut_i& attachment );

		bool InAttached() const;
		bool OutAttached() const;

		ChannelAttachmentIn_i& InAttachment() const;
		ChannelAttachmentOut_i& OutAttachment() const;

		void Close( Host& host );
		bool IsClosed() const;

	private:
		ChannelAttachmentIn_i* m_pAttachmentIn;
		ChannelAttachmentOut_i* m_pAttachmentOut;
		bool m_isClosed;
	};

	class OpChannel : public Operation
	{
	public:
		OpChannel();
		virtual ~OpChannel();

		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );

	protected:
		void Communicate( Host& host, Process& inputProcess );

		bool InitChannel( lua::LuaStack& args, InitError& initError );
		void UnrefChannel( lua::LuaStack const& stack );

		void InitArguments( lua::LuaStack& args, InitError& initError );
		void SetArguments( ChannelArgument* arguments, int numArguments );
		void UnrefArguments( lua::LuaStack const& stack );

		bool HasChannel();
		Channel& ThisChannel();

		void MoveChannelArguments( ChannelArgument* arguments, int numArguments );
		void ArgumentsMoved();		
		ChannelArgument* Arguments() const;
		int NumArguments() const;
		bool HasArgumentsMoved() const;
		bool HasArguments() const;

		virtual void Terminate( Host& host );
		void CloseChannel( Host& host, Channel& channel );

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

	protected:
		virtual int PushResults( lua::LuaStack& luaStack );

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual void Terminate( Host& host );
		
		virtual Process& ProcessToEvaluate();
		virtual void MoveChannelArguments( Channel& channel, ChannelArgument* arguments, int numArguments );
		virtual void CloseChannel( Host& host, Channel& channel );
	};

	class OpChannelOut : public OpChannel, ChannelAttachmentOut_i
	{
	public:
		OpChannelOut();
		virtual ~OpChannelOut();

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual void Terminate( Host& host );

		virtual Process& ProcessToEvaluate();
		virtual void Communicate( Host& host, Process& inputProcess );
		virtual void CloseChannel( Host& host, Channel& channel );
	};

	class OpChannelRange : public OpChannelIn
	{
	public:
		OpChannelRange();
		virtual ~OpChannelRange();

	private:
		virtual int PushResults( lua::LuaStack& luaStack );
	};

	void PushChannel( lua_State* luaState, Channel& channel );
	bool IsChannelArg( lua::LuaStackValue const& value );
	Channel* GetChannelArg( lua::LuaStackValue const& value );

	void InitializeChannels( lua::LuaState& state );
	void ShutdownChannels( lua::LuaState& state );
}
