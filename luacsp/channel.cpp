#include "channel.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

namespace csp
{
	void PushGcObject( lua_State* luaState, GcObject& gcObject, void* metatableRegistryKey );
	int channel( lua_State* luaState );

	int GcObject_Gc( lua_State* luaState );
	int Channel_IN( lua_State* luaState );
	int Channel_OUT( lua_State* luaState );

	const csp::FunctionRegistration channelGlobals[] =
	{
		"Channel", csp::channel
		, NULL, NULL
	};

	const csp::FunctionRegistration channelFunctions[] =
	{
		"__gc", csp::GcObject_Gc
		, "IN", csp::Channel_IN
		, "OUT", csp::Channel_OUT
		, NULL, NULL
	};
}

csp::OpChannel::OpChannel()
	: m_pChannel()
	, m_channelRefKey( lua::LUA_NO_REF )
	, m_arguments()
	, m_numArguments( 0 )
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
		return initError.ArgError( 1, "channel value expected" );

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

csp::WorkResult::Enum csp::OpChannel::Work( Host&, time_t )
{
	return WorkResult::YIELD;
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

void csp::OpChannel::MoveChannelArguments( ChannelArgument* arguments, int numArguments )
{
	CORE_ASSERT( m_arguments == NULL );
	CORE_ASSERT( m_numArguments == 0 );

	m_arguments = arguments;
	m_numArguments = numArguments;

	m_argumentsMoved = true;
}

void csp::OpChannel::ArgumentsMoved()
{
	m_arguments = NULL;
	m_numArguments = 0;

	m_argumentsMoved = true;
}

csp::ChannelArgument* csp::OpChannel::Arguments() const
{
	return m_arguments;
}

int csp::OpChannel::NumArguments() const
{
	return m_numArguments;
}

bool csp::OpChannel::HaveArgumentsMoved() const
{
	return m_argumentsMoved;
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

	if( HaveArgumentsMoved() )
	{
		UnrefChannel( host.LuaState().GetStack() );
		return WorkResult::FINISH;
	}

	if( channel.InAttached() )
	{
		ChannelAttachmentIn_i& in = channel.InAttachment();

		MoveChannelArguments();

		host.PushEvalStep( ThisProcess() );
		host.PushEvalStep( in.ProcessToEvaluate() );
	}

	return WorkResult::YIELD;
}

csp::Process& csp::OpChannelOut::ProcessToEvaluate()
{
	return ThisProcess();
}

void csp::OpChannelOut::MoveChannelArguments()
{
	Channel& channel = ThisChannel();
	ChannelAttachmentIn_i& in = channel.InAttachment();
	
	in.MoveChannelArguments( channel, Arguments(), NumArguments() );
	ArgumentsMoved();	

	channel.SetAttachmentOut( NULL );
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

	if( HaveArgumentsMoved() )
	{
		UnrefChannel( host.LuaState().GetStack() );
		return WorkResult::FINISH;
	}
		
	if( channel.OutAttached() )
	{
		ChannelAttachmentOut_i& out = channel.OutAttachment();
		out.MoveChannelArguments();

		host.PushEvalStep( out.ProcessToEvaluate() );
		host.PushEvalStep( ThisProcess() );
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
	ChannelArgument* arguments = Arguments();
	int numArguments = NumArguments();

	for( int i = 0; i < numArguments; ++i )
	{
		luaStack.PushRegistryReferenced( arguments[i].refKey );
	}

	UnrefArguments( luaStack );
	return numArguments;
}


csp::Channel::Channel()
	: m_pAttachmentIn()
	, m_pAttachmentOut()
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


int csp::GcObject_Gc( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	CORE_ASSERT( args.NumArgs() == 1 );
	lua::LuaStackValue userData = args[1];

	CORE_ASSERT( userData.IsUserData() );
	GcObject** pGcObject = (GcObject**)userData.GetUserData();

	delete *pGcObject;
	*pGcObject = NULL;
	return 0;
}

int csp::Channel_IN( lua_State* luaState )
{
	OpChannelIn* pIn = CORE_NEW OpChannelIn();
	return pIn->Initialize( luaState );
}

int csp::Channel_OUT( lua_State* luaState )
{
	OpChannelOut* pOut = CORE_NEW OpChannelOut();
	return pOut->Initialize( luaState );
}

void csp::PushGcObject( lua_State* luaState, GcObject& gcObject, void* metatableRegistryKey )
{
	lua::LuaStack args( luaState );

	GcObject** ppGcObject = (GcObject**)args.PushUserData( sizeof(GcObject*) );
	*ppGcObject = &gcObject;
	lua::LuaStackValue userData = args.GetTopValue();

	args.PushLightUserData( metatableRegistryKey );
	lua::LuaStackValue metatable = args.RegistryGet();
	CORE_ASSERT( metatable.IsTable() );

	args.SetMetaTable( userData );
}

void csp::PushChannel( lua_State* luaState, Channel& ch )
{
	PushGcObject( luaState, ch, (void*)channelFunctions );
}

bool csp::IsChannelArg( lua::LuaStackValue const& value )
{
	return value.IsUserData(); //TODO: add more checks.
}

csp::Channel* csp::GetChannelArg( lua::LuaStackValue const& value )
{
	if( !value.IsUserData() )
		return NULL;

	GcObject** pGcObject = (GcObject**)value.GetUserData();
	Channel* pChannel = static_cast< Channel* >( *pGcObject ); //TODO: add more type checks.
	return pChannel;
}


int csp::channel( lua_State* luaState )
{
	csp::Channel* pChannel = CORE_NEW csp::Channel();
	csp::PushChannel( luaState, *pChannel );
	return 1;
}

void csp::InitializeChannels( lua::LuaState& state )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)channelFunctions );
	lua::LuaStackValue metaTable = stack.PushTable();

	metaTable.PushValue();
	stack.SetField( metaTable, "__index" );
	RegisterFunctions( state, metaTable, channelFunctions );

	stack.RegistrySet();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	RegisterFunctions( state, globals, channelGlobals );
	stack.Pop(1);
}

void csp::ShutdownChannels( lua::LuaState& state )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)channelFunctions );
	stack.PushNil();

	stack.RegistrySet();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	UnregisterFunctions( state, globals, channelGlobals );
	stack.Pop(1);
}
