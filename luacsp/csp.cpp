/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "csp.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

namespace csp
{
	void PushGcObject( lua_State* luaState, GcObject& gcObject, void* metatableRegistryKey );
}

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


lua::LuaStackValue csp::PushCspMetatable( lua_State* luaState, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack stack( luaState );

	stack.PushLightUserData( (void*)memberFunctions );
	lua::LuaStackValue metatable = stack.RegistryGet();
	CORE_ASSERT( metatable.IsTable() );
	return metatable;
}

void csp::CspSetMetatable( lua_State* luaState, const lua::LuaStackValue& value, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack stack( luaState );
	PushCspMetatable( luaState, memberFunctions );
	stack.SetMetaTable( value );
}

bool csp::CspHasMetatable( lua_State* luaState, const lua::LuaStackValue& value, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack stack( luaState );
	
	lua::LuaStackValue memberMetatable = PushCspMetatable( luaState, memberFunctions );

	if( !stack.GetMetaTable( value ) )
	{
		stack.Pop( 1 );
		return false;
	}

	lua::LuaStackValue metatable = stack.GetTopValue();

	bool result = metatable.IsEqualByRef( memberMetatable );
	
	stack.Pop( 2 );
	return result;
}

void csp::PushGcObject( lua_State* luaState, GcObject& gcObject, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack args( luaState );

	GcObject** ppGcObject = (GcObject**)args.PushUserData( sizeof(GcObject*) );
	*ppGcObject = &gcObject;
	lua::LuaStackValue userData = args.GetTopValue();

	return CspSetMetatable( luaState, userData, memberFunctions );
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

void csp::InitilaizeCspObjectGlobals( lua::LuaState& state, const FunctionRegistration globalFunctions[]
	, const char* scopeName )
{
	lua::LuaStack stack = state.GetStack();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	lua::LuaStackValue scope = stack.PushTable();
	RegisterFunctions( state, scope, globalFunctions );
	stack.SetField( globals, scopeName );
	stack.Pop(1);
}

void csp::InitializeCspObject( lua::LuaState& state, const char* scopeName, const FunctionRegistration globalFunctions[]
	, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)memberFunctions );
	lua::LuaStackValue metaTable = stack.PushTable();

	metaTable.PushValue();
	stack.SetField( metaTable, "__index" );
	RegisterFunctions( state, metaTable, memberFunctions );

	stack.RegistrySet();

	InitilaizeCspObjectGlobals( state, globalFunctions, scopeName );
}

void csp::InitializeCspObjectEnv( lua::LuaState& state, const char* scopeName, const FunctionRegistration globalFunctions[]
	, const FunctionRegistration memberFunctions[], lua::LuaStackValue& env )
{
	lua::LuaStack stack = state.GetStack();

	lua::LuaStackValue functionsTable = stack.PushTable();
	RegisterFunctions( state, functionsTable, memberFunctions );

	lua::LuaStackValue functionsMetaTable = stack.PushTable();
	env.PushValue();
	stack.SetField( functionsMetaTable, "__index" );
	stack.SetMetaTable( functionsTable );

	stack.PushLightUserData( (void*)memberFunctions ); // registry key

	lua::LuaStackValue objectMetaTable = stack.PushTable(); // registry value
	functionsTable.PushValue();
	stack.SetField( objectMetaTable, "__index" );
	
	stack.RegistrySet(); // set key, value.

	stack.Pop(1); // pop functionsTable.

	InitilaizeCspObjectGlobals( state, globalFunctions, scopeName );
}

void csp::ShutdownCspObject( lua::LuaState& state, const char* scopeName, const FunctionRegistration[]
	, const FunctionRegistration memberFunctions[] )
{
	lua::LuaStack stack = state.GetStack();

	stack.PushLightUserData( (void*)memberFunctions );
	stack.PushNil();

	stack.RegistrySet();

	lua::LuaStackValue globals = stack.PushGlobalTable();
	stack.PushNil();
	stack.SetField( globals, scopeName );
	stack.Pop(1);
}


csp::GcObject::GcObject()
{
}

csp::GcObject::~GcObject()
{
}
