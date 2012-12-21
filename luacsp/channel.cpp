#include "channel.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

namespace csp
{
	int Channel_new( lua_State* luaState );
	int GcObject_Gc( lua_State* luaState );
	int Channel_IN( lua_State* luaState );
	int Channel_OUT( lua_State* luaState );

	int Channel_RANGE( lua_State* luaState );
	int RANGE_next( lua_State* luaState );
	int Channel_close( lua_State* luaState );
	
	const csp::FunctionRegistration channelGlobals[] =
	{
		"new", csp::Channel_new
		, NULL, NULL
	};

	const csp::FunctionRegistration channelFunctions[] =
	{
		"__gc", csp::GcObject_Gc
		, "IN", csp::Channel_IN
		, "OUT", csp::Channel_OUT
		, "RANGE", csp::Channel_RANGE
		, "close", csp::Channel_close
		, NULL, NULL
	};
}

csp::OpChannel::OpChannel()
	: m_pChannel()
	, m_channelRefKey( lua::LUA_NO_REF )
	, m_arguments()
	, m_numArguments( CSP_NO_ARGS )
	, m_argumentsMoved( false )
{
}

csp::OpChannel::~OpChannel()
{
	CORE_ASSERT( m_channelRefKey == lua::LUA_NO_REF );

	for( int i = 0; i < m_numArguments; ++i )
	{
		CORE_ASSERT( m_arguments[i].refKey == lua::LUA_NO_REF );
	}

	delete[] m_arguments;
	m_arguments = NULL;
}

bool csp::OpChannel::InitChannel( lua::LuaStack& args, InitError& initError )
{
	lua::LuaStackValue channelArg = args[1];
	if( !IsChannelArg( channelArg ) )
		return initError.ArgError( 1, "channel expected" );

	Channel* pChannel = GetChannelArg( channelArg );
	if( pChannel == NULL )
		return initError.ArgError( 1, "channel pointer expected" );

	channelArg.PushValue();
	m_channelRefKey = args.RefInRegistry();

	m_pChannel = pChannel;
	return true;
}

void csp::OpChannel::InitArguments( lua::LuaStack& args, InitError& )
{
	m_numArguments = args.NumArgs() - 1;
	if ( m_numArguments > 0 )
		m_arguments = CORE_NEW ChannelArgument[ m_numArguments ];

	for( int i = 2; i <= args.NumArgs(); ++i )
	{
		args[ i ].PushValue();
		m_arguments[ i-2 ].refKey = args.RefInRegistry();
	}
}

csp::WorkResult::Enum csp::OpChannel::Work( Host&, CspTime_t )
{
	return WorkResult::YIELD;
}

void csp::OpChannel::Communicate( Host& host, Process& inputProcess )
{
	Channel& channel = ThisChannel();
	ChannelAttachmentIn_i& in = channel.InAttachment();

	in.MoveChannelArguments( channel, Arguments(), NumArguments() );
	ArgumentsMoved();	

	channel.SetAttachmentOut( NULL );

	host.PushEvalStep( ThisProcess() );
	host.PushEvalStep( inputProcess );
}

void csp::OpChannel::UnrefChannel( lua::LuaStack const& stack )
{
	m_pChannel = NULL;
	stack.UnrefInRegistry( m_channelRefKey );
	m_channelRefKey = lua::LUA_NO_REF;
}

csp::Channel& csp::OpChannel::ThisChannel()
{
	CORE_ASSERT( m_pChannel );
	return *m_pChannel;
}

bool csp::OpChannel::HasChannel()
{
	return m_pChannel != NULL;
}

void csp::OpChannel::UnrefArguments( lua::LuaStack const& stack )
{
	for( int i = 0; i < m_numArguments; ++i )
	{
		stack.UnrefInRegistry( m_arguments[i].refKey );
		m_arguments[i].refKey = lua::LUA_NO_REF;
	}
}


void csp::OpChannel::SetArguments( ChannelArgument* arguments, int numArguments )
{
	CORE_ASSERT( m_arguments == NULL );
	CORE_ASSERT( m_numArguments == CSP_NO_ARGS );
	CORE_ASSERT( numArguments != CSP_NO_ARGS );

	m_arguments = arguments;
	m_numArguments = numArguments;
}

void csp::OpChannel::MoveChannelArguments( ChannelArgument* arguments, int numArguments )
{
	SetArguments( arguments, numArguments );
	m_argumentsMoved = true;
}

void csp::OpChannel::ArgumentsMoved()
{
	m_arguments = NULL;
	m_numArguments = CSP_NO_ARGS;

	m_argumentsMoved = true;
}

csp::ChannelArgument* csp::OpChannel::Arguments() const
{
	CORE_ASSERT( HasArguments() );
	return m_arguments;
}

int csp::OpChannel::NumArguments() const
{
	CORE_ASSERT( HasArguments() );
	return m_numArguments;
}

bool csp::OpChannel::HasArgumentsMoved() const
{
	return m_argumentsMoved;
}

bool csp::OpChannel::HasArguments() const
{
	return m_numArguments != CSP_NO_ARGS;
}

void csp::OpChannel::Terminate( Host& host )
{
	lua::LuaStack& stack = host.LuaState().GetStack();
	UnrefChannel( stack );
	UnrefArguments( stack );
}

void csp::OpChannel::CloseChannel( Host& host, Channel& )
{
	lua::LuaStack& stack = host.LuaState().GetStack();
	UnrefArguments( stack );
	ArgumentsMoved();
}


csp::OpChannelOut::OpChannelOut()
{
}

csp::OpChannelOut::~OpChannelOut()
{
}

bool csp::OpChannelOut::Init( lua::LuaStack & args, InitError& initError )
{
	if( !InitChannel( args, initError ) )
		return false;

	InitArguments( args, initError );

	if( ThisChannel().OutAttached() )
		return initError.ArgError( 1, "channel is in output operation already" );

	ThisChannel().SetAttachmentOut( this );
	return true;
}

csp::WorkResult::Enum csp::OpChannelOut::Evaluate( Host& host )
{
	Channel& channel = ThisChannel();

	if( HasArgumentsMoved() )
	{
		UnrefChannel( host.LuaState().GetStack() );
		return WorkResult::FINISH;
	}

	if( channel.InAttached() )
	{
		ChannelAttachmentIn_i& in = channel.InAttachment();
		Communicate( host, in.ProcessToEvaluate() );
	}

	return WorkResult::YIELD;
}

csp::Process& csp::OpChannelOut::ProcessToEvaluate()
{
	return ThisProcess();
}

void csp::OpChannelOut::Communicate( Host& host, Process& inputProcess )
{
	OpChannel::Communicate( host, inputProcess );
}

void csp::OpChannelOut::Terminate( Host& host )
{
	ThisChannel().SetAttachmentOut( NULL );
	OpChannel::Terminate( host );
}

void csp::OpChannelOut::CloseChannel( Host& host, Channel& channel )
{
	OpChannel::CloseChannel( host, channel );
}


csp::OpChannelIn::OpChannelIn()
{
}

csp::OpChannelIn::~OpChannelIn()
{
}

bool csp::OpChannelIn::Init( lua::LuaStack & args, InitError& initError )
{
	if( !InitChannel( args, initError ) )
		return false;

	if( ThisChannel().InAttached() )
		return initError.ArgError( 1, "channel is in input operation already" );

	ThisChannel().SetAttachmentIn( this );
	return true;
}

csp::WorkResult::Enum csp::OpChannelIn::Evaluate( Host& host )
{
	Channel& channel = ThisChannel();

	if( HasArgumentsMoved() )
	{
		return WorkResult::FINISH;
	}
		
	if( channel.OutAttached() )
	{
		ChannelAttachmentOut_i& out = channel.OutAttachment();
		out.Communicate( host, ThisProcess() );
	}

	return WorkResult::YIELD;
}

csp::Process& csp::OpChannelIn::ProcessToEvaluate()
{
	return ThisProcess();
}

void csp::OpChannelIn::MoveChannelArguments( Channel&, ChannelArgument* arguments, int numArguments )
{
	OpChannel::MoveChannelArguments( arguments, numArguments );
	ThisChannel().SetAttachmentIn( NULL );
}

int csp::OpChannelIn::PushResults( lua::LuaStack & luaStack )
{
	int numArguments = 0;
	if( HasArguments() )
	{
		ChannelArgument* arguments = Arguments();
		numArguments = NumArguments();

		for( int i = 0; i < numArguments; ++i )
			luaStack.PushRegistryReferenced( arguments[i].refKey );
	}

	UnrefArguments( luaStack );
	UnrefChannel( luaStack );
	return numArguments;
}

void csp::OpChannelIn::Terminate( Host& host )
{
	ThisChannel().SetAttachmentIn( NULL );
	OpChannel::Terminate( host );
}

void csp::OpChannelIn::CloseChannel( Host& host, Channel& channel )
{
	OpChannel::CloseChannel( host, channel );
}


csp::OpChannelRange::OpChannelRange()
{
}

csp::OpChannelRange::~OpChannelRange()
{
}

int csp::OpChannelRange::PushResults( lua::LuaStack & luaStack )
{
	if( ThisChannel().IsClosed() )
		luaStack.PushNil();
	else
		luaStack.PushBoolean( true );
	return OpChannelIn::PushResults( luaStack ) + 1;
}


csp::Channel::Channel()
	: m_pAttachmentIn()
	, m_pAttachmentOut()
	, m_isClosed( false )
{
}

csp::Channel::~Channel()
{
	CORE_ASSERT( m_pAttachmentIn == NULL );
	CORE_ASSERT( m_pAttachmentOut == NULL );
}

void csp::Channel::SetAttachmentIn( ChannelAttachmentIn_i* pAttachment )
{
	m_pAttachmentIn = pAttachment;
}

void csp::Channel::SetAttachmentOut( ChannelAttachmentOut_i* pAttachment )
{
	m_pAttachmentOut = pAttachment;
}

bool csp::Channel::InAttached() const
{
	return m_pAttachmentIn != NULL;
}

bool csp::Channel::OutAttached() const
{
	return m_pAttachmentOut != NULL;
}

csp::ChannelAttachmentIn_i& csp::Channel::InAttachment() const
{
	CORE_ASSERT( m_pAttachmentIn );
	return *m_pAttachmentIn;
}

csp::ChannelAttachmentOut_i& csp::Channel::OutAttachment() const
{
	CORE_ASSERT( m_pAttachmentOut );
	return *m_pAttachmentOut;
}

bool csp::Channel::IsClosed() const
{
	return m_isClosed;
}

void csp::Channel::Close( Host& host )
{
	m_isClosed = true;

	if( InAttached() )
	{
		InAttachment().CloseChannel( host, *this );
		host.PushEvalStep( InAttachment().ProcessToEvaluate() );
		SetAttachmentIn( NULL );
	}
	if( OutAttached() )
	{
		OutAttachment().CloseChannel( host, *this );
		host.PushEvalStep( OutAttachment().ProcessToEvaluate() );
		SetAttachmentOut( NULL );
	}
}


int csp::Channel_new( lua_State* luaState )
{
	csp::Channel* pChannel = CORE_NEW csp::Channel();
	csp::PushChannel( luaState, *pChannel );
	return 1;
}

int csp::Channel_IN( lua_State* luaState )
{
	OpChannelIn* pIn = CORE_NEW OpChannelIn();
	return pIn->DoInit( luaState );
}

int csp::Channel_OUT( lua_State* luaState )
{
	OpChannelOut* pOut = CORE_NEW OpChannelOut();
	return pOut->DoInit( luaState );
}

int csp::Channel_RANGE( lua_State* luaState )
{
	lua::LuaStack stack( luaState );
	lua::LuaStackValue channel = stack[1];
	if( !csp::IsChannelArg( channel ) )
		return channel.ArgError( "Channel expected." );

	stack.PushCFunction( RANGE_next );
	channel.PushValue();
	stack.PushBoolean( true );
	return 3;
}

int csp::RANGE_next( lua_State* luaState )
{
	OpChannelRange* pRange = CORE_NEW OpChannelRange();
	return pRange->DoInit( luaState );
}

int csp::Channel_close( lua_State* luaState )
{
	lua::LuaStack stack( luaState );
	lua::LuaStackValue channel = stack[1];
	if( !csp::IsChannelArg( channel ) )
		return channel.ArgError( "Channel expected." );

	Channel* pChannel = GetChannelArg( channel );
	if( pChannel == NULL )
		return channel.ArgError( "Channel pointer expected." );

	Host& host = Host::GetHost( luaState );
	pChannel->Close( host );
	return 0;
}

void csp::PushChannel( lua_State* luaState, Channel& ch )
{
	PushGcObject( luaState, ch, channelFunctions );
}

bool csp::IsChannelArg( lua::LuaStackValue const& value )
{
	return value.IsUserData(); // TODO: add more checks.
}

csp::Channel* csp::GetChannelArg( lua::LuaStackValue const& value )
{
	if( !value.IsUserData() )
		return NULL;

	GcObject** pGcObject = (GcObject**)value.GetUserData();
	Channel* pChannel = static_cast< Channel* >( *pGcObject ); // TODO: add more type checks.
	return pChannel;
}


void csp::InitializeChannels( lua::LuaState& state )
{
	InitializeCspObject( state, "Channel", channelGlobals, channelFunctions );
}

void csp::ShutdownChannels( lua::LuaState& state )
{
	ShutdownCspObject( state, "Channel", channelGlobals, channelFunctions );
}
