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
			int numArguments = CSP_NO_ARGS;
			ChannelArgument* arguments = CreateArguments( host.LuaState().GetStack(), numArguments );
			SetArguments( arguments, numArguments );
			AttachOutput();
		}
	}

	if( IsOutputAttached() )
	{
		if( channel.InAttached() )
		{
			ChannelAttachmentIn_i& in = channel.InAttachment();

			MoveChannelArguments();

			host.PushEvalStep( ThisProcess() );
			host.PushEvalStep( in.ProcessToEvaluate() );
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
			DetachOutput();
		return result;
	}

	return Evaluate( host );
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

void csp::OpCppChannelOut::MoveChannelArguments()
{
	Channel& channel = ThisChannel();
	ChannelAttachmentIn_i& in = channel.InAttachment();

	in.MoveChannelArguments( channel, Arguments(), NumArguments() );
	ArgumentsMoved();	

	CORE_ASSERT( IsOutputAttached() );
	DetachOutput();
}

bool csp::OpCppChannelOut::AttachOutput()
{
	Channel& channel = ThisChannel();
	
	if( channel.OutAttached() )
		return false;

	ThisChannel().SetAttachmentOut( this );
	return true;
}

bool csp::OpCppChannelOut::DetachOutput()
{
	Channel& channel = ThisChannel();
	
	if( !channel.OutAttached() )
		return false;

	if( !IsOutputAttached() )
		return false;

	ThisChannel().SetAttachmentOut( NULL );
	return true;
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
