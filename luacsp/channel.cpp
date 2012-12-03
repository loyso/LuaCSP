#include <luacpp/luacpp.h>

#include "channel.h"

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
	return 0;
}

int csp::Channel_OUT( lua_State* luaState )
{
	return 0;
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
