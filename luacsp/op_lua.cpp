/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "op_lua.h"

#include "luacpp/luastack.h"
#include "luacpp/luastackvalue.h"

#include "host.h"

namespace csp
{
	int CspOperation_table( lua_State* luaState );

	int OpLua_new( lua_State* luaState );
	int OpLua_Init( lua_State* luaState );

	const csp::FunctionRegistration cspOperationGlobals[] =
	{
		  "table", csp::CspOperation_table
		, NULL, NULL
	};

	const csp::FunctionRegistration cspOperationFunctions[] =
	{
		  "new", csp::OpLua_new
		, "Init", csp::OpLua_Init
		, NULL, NULL
	};
}


csp::OpLua::OpLua()
	: m_self( lua::LUA_NO_REF )
{
}

csp::OpLua::~OpLua()
{
	CORE_ASSERT( m_self == lua::LUA_NO_REF );
}


bool csp::OpLua::Init( lua::LuaStack& args, InitError& initError )
{
	lua::LuaStackValue self = args[1];
	if( !self.IsTable() )
		return initError.ArgError( 1, "OpLua table expected." );

	self.PushValue();
	m_self = args.RefInRegistry();

	return true;
}

csp::WorkResult::Enum csp::OpLua::Work( Host& host, CspTime_t dt )
{
	lua::LuaStack& stack = host.LuaState().GetStack();

	stack.PushNumber( dt );
	if ( !CallLua( stack, "Work", 1, 1 ) )
		return WorkResult::FINISH;

	WorkResult::Enum workResult = WorkResult::FINISH;

	lua::LuaStackValue callResult = stack.GetTopValue();
	if( callResult.IsNumber() )
	{
		workResult = (WorkResult::Enum)callResult.GetInteger();
	}
		
	stack.Pop( 1 );
	return workResult;
}

int csp::OpLua::PushResults( lua::LuaStack& stack )
{
	int numResults = 0;

	int top = stack.GetTop();

	if ( CallLua( stack, "PushResults", 0, lua::LUA_MULT_RET ) )
		numResults = stack.GetTop() - top;

	UnrefSelf( stack );
	return numResults;
}

void csp::OpLua::Terminate( Host& host )
{
	lua::LuaStack& stack = host.LuaState().GetStack();
	CallLua( stack, "Terminate", 0, 0 );
	UnrefSelf( stack );
}

bool csp::OpLua::CallLua( lua::LuaStack& stack, const char* functionName, int numArgs, int numRets )
{
	CORE_ASSERT( m_self != lua::LUA_NO_REF );
	stack.PushRegistryReferenced( m_self );

	lua::LuaStackValue top = stack.GetTopValue();
	CORE_ASSERT( top.IsTable() );

	stack.GetField( top, functionName );

	if ( !stack.GetTopValue().IsFunction() )
	{
		stack.Pop( numArgs + 2 );
		return false;
	}

	// stack at this point: arg1, arg2, argN, self, function

	stack.Insert( -numArgs-2 );
	stack.Insert( -numArgs-1 );

	// stack at this point: function, self, arg1, arg2, argN

	lua::LuaState luaState( stack.InternalState() );
	lua::Return::Enum result = luaState.Call( 1+numArgs, numRets );
	return result == lua::Return::OK;
}

void csp::OpLua::UnrefSelf( lua::LuaStack& luaStack )
{
	CORE_ASSERT( m_self != lua::LUA_NO_REF );
	luaStack.UnrefInRegistry( m_self );
	m_self = lua::LUA_NO_REF;
}

int csp::CspOperation_table( lua_State* luaState )
{
	lua::LuaStack stack( luaState );
	lua::LuaStackValue table = stack.PushTable();

	table.PushValue();
	stack.SetField( table, "__index" );

	CspSetMetatable( luaState, stack.GetTopValue(), cspOperationFunctions );
	return 1;
}

int csp::OpLua_new( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	lua::LuaStackValue self = stack[1];
	if( !self.IsTable() )
		return self.ArgError( "CspOperation table expected." );

	lua::LuaStackValue operation = stack.PushTable();

	self.PushValue();
	stack.SetMetaTable( operation );

	return 1;
}

int csp::OpLua_Init( lua_State* luaState )
{
	OpLua* pOpLua = CORE_NEW OpLua();
	return pOpLua->DoInit( luaState );
}

void csp::InitializeOpLua( lua::LuaState& state )
{
	InitializeCspObject( state, "CspOperation", cspOperationGlobals, cspOperationFunctions );
}

void csp::ShutdownOpLua( lua::LuaState& state )
{
	ShutdownCspObject( state, "CspOperation", cspOperationGlobals, cspOperationFunctions );
}
