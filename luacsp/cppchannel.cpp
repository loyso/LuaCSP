/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "cppchannel.h"

#include <luacpp/luastackvalue.h>

#include "host.h"

csp::OpCppChannelOut::OpCppChannelOut()
{
}

csp::OpCppChannelOut::~OpCppChannelOut()
{
}

bool csp::OpCppChannelOut::Init( lua::LuaStack& args, InitError& initError )
{
	if( !InitChannel( args, initError ) )
		return false;

	return true;
}

csp::WorkResult::Enum csp::OpCppChannelOut::Evaluate( Host& host )
{
	Channel& channel = ThisChannel();

	if( !IsOutputAttached() )
	{
		if( IsOutputReady() )
		{
			lua::LuaStack& stack = host.LuaState().GetStack();
			MemorizeOutputArguments( stack );
			ThisChannel().SetAttachmentOut( *this );
		}
	}

	if( IsOutputAttached() )
	{
		if( channel.InAttached() )
		{
			ChannelAttachmentIn_i& in = channel.InAttachment();
			Communicate( host, in.ProcessToEvaluate() );
		}
	}

	return WorkResult::YIELD;
}

csp::WorkResult::Enum csp::OpCppChannelOut::Work( Host& host, CspTime_t dt )
{
	WorkResult::Enum result = Update( dt );
	if( result == WorkResult::FINISH )
	{
		if( IsOutputAttached() )
			ThisChannel().ResetAttachmentOut( *this );
		return result;
	}

	return Evaluate( host );
}

void csp::OpCppChannelOut::MemorizeOutputArguments( lua::LuaStack& stack )
{
	int numArguments = PushOutputArguments( stack );

	csp::ChannelArgument* arguments = CORE_NEW csp::ChannelArgument[ numArguments ];
	for( int i = numArguments-1; i >= 0; --i )
		arguments[ i ].refKey = stack.RefInRegistry();

	SetArguments( arguments, numArguments );
}

int csp::OpCppChannelOut::PushResults( lua::LuaStack& luaStack )
{
	UnrefChannel( luaStack );
	UnrefArguments( luaStack );
	return 0;
}

void csp::OpCppChannelOut::Terminate( Host& host )
{
	OpChannel::Terminate( host );
}

csp::Process& csp::OpCppChannelOut::ProcessToEvaluate()
{
	return ThisProcess();
}

void csp::OpCppChannelOut::Communicate( Host& host, Process& inputProcess )
{
	CORE_ASSERT( IsOutputAttached() );
	OpChannel::Communicate( host, inputProcess );
	ThisChannel().ResetAttachmentOut( *this );
}

void csp::OpCppChannelOut::CloseChannel( Host& host, Channel& channel )
{
	OpChannel::CloseChannel( host, channel );
	channel.ResetAttachmentOut( *this );
}

bool csp::OpCppChannelOut::IsOutputAttached()
{
	Channel& channel = ThisChannel();
	return channel.OutAttached() && &channel.OutAttachment() == this;
}

void csp::InitializeCppChannels( lua::LuaState& )
{
}

void csp::ShutdownCppChannels( lua::LuaState& )
{
}
