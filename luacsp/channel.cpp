#include <luacpp/luacpp.h>

#include "channel.h"
#include "host.h"

#include <stdlib.h>
#include <assert.h>

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

csp::GcObject::GcObject()
{
}

csp::GcObject::~GcObject()
{
}

int csp::GcObject_Gc( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	assert( args.NumArgs() == 1 );
	lua::LuaStackValue userData = args[1];

	assert( userData.IsUserData() );
	GcObject** pGcObject = (GcObject**)userData.GetUserData();

	delete *pGcObject;
	*pGcObject = NULL;
	return 0;
}

int csp::Channel_IN( lua_State* luaState )
{
	OpChannelIn* pIn = new OpChannelIn();
	return pIn->Initialize( luaState );
}

int csp::Channel_OUT( lua_State* luaState )
{
	OpChannelOut* pOut = new OpChannelOut();
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
	assert( metatable.IsTable() );

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
	csp::Channel* pChannel = new csp::Channel();
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


csp::OpChannel::OpChannel()
	: m_pChannel()
	, m_channelRefKey( lua::LUA_NO_REF )
{
}

csp::OpChannel::~OpChannel()
{
	assert( m_channelRefKey == lua::LUA_NO_REF );
}

bool csp::OpChannel::Init( lua::LuaStack& args )
{
	lua::LuaStackValue channelArg = args[1];
	if( !IsChannelArg( channelArg ) )
	{
		channelArg.ArgError( "channel expected" );
		return false;
	}

	Channel* pChannel = GetChannelArg( channelArg );
	if( pChannel == NULL )
	{
		channelArg.ArgError( "channel value expected" );
		return false;
	}

	m_channelRefKey = args.RefInRegistry();
	m_pChannel = pChannel;
	return true;
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


csp::OpChannelOut::OpChannelOut()
{
}

csp::OpChannelOut::~OpChannelOut()
{
}

csp::WorkResult::Enum csp::OpChannelOut::Evaluate( Host& host )
{
	UnrefChannel( host.LuaState().GetStack() );
	return WorkResult::FINISH;
}


csp::OpChannelIn::OpChannelIn()
{
}

csp::OpChannelIn::~OpChannelIn()
{
}

csp::WorkResult::Enum csp::OpChannelIn::Evaluate( Host& host )
{
	UnrefChannel( host.LuaState().GetStack() );
	return WorkResult::FINISH;
}

