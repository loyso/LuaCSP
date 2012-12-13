
#include "csp.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

csp::Host& csp::Initialize()
{
    lua::LuaState luaState = lua::LuaState::NewState();
    
	csp::Host& host = *new csp::Host(luaState);
    host.Initialize();
    
	return host;
}

void csp::Shutdown(csp::Host& host)
{
	lua::LuaState luaState = host.LuaState();
    
	host.Shutdown();
    delete &host;

	lua::LuaState::CloseState( luaState );
}

void csp::RegisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration descriptions[] )
{
	lua::LuaStack stack = state.GetStack();
	for( int i = 0; descriptions[i].function; ++i)
	{
		stack.PushCFunction( descriptions[i].function );
		stack.SetField( value, descriptions[i].name );
	}
}

void csp::UnregisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration descriptions[] )
{
	lua::LuaStack stack = state.GetStack();
	for( int i = 0; descriptions[i].function; ++i)
	{
		stack.PushNil();
		stack.SetField( value, descriptions[i].name );
	}
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

void csp::PushGcObject( lua_State* luaState, GcObject& gcObject, const FunctionRegistration memberFunctions[] )
{
	PushGcObject( luaState, gcObject, (void*)memberFunctions );
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

void csp::InitializeCspObject( lua::LuaState& state, const FunctionRegistration memberFunctions[], const FunctionRegistration globalFunctions[] )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)memberFunctions );
	lua::LuaStackValue metaTable = stack.PushTable();

	metaTable.PushValue();
	stack.SetField( metaTable, "__index" );
	RegisterFunctions( state, metaTable, memberFunctions );

	stack.RegistrySet();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	RegisterFunctions( state, globals, globalFunctions );
	stack.Pop(1);
}


void csp::ShutdownCspObject( lua::LuaState& state, const FunctionRegistration memberFunctions[], const FunctionRegistration globalFunctions[] )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)memberFunctions );
	stack.PushNil();

	stack.RegistrySet();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	UnregisterFunctions( state, globals, globalFunctions );
	stack.Pop(1);
}


csp::GcObject::GcObject()
{
}

csp::GcObject::~GcObject()
{
}


