/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "luastackvalue.h"

extern "C"
{
#include <lua/src/lua.h>
#include <lua/src/lauxlib.h>
}

#include <string.h>

lua::LuaStackValue::LuaStackValue()
	: m_state( NULL )
	, m_index( 0 )
{
}

lua::LuaStackValue::LuaStackValue( lua_State* luaState, int index )
	: m_state( luaState )
	, m_index( index )
{
}

bool lua::LuaStackValue::IsNil() const
{
	return lua_isnil(m_state, m_index);
}

bool lua::LuaStackValue::IsFunction() const
{
	return lua_isfunction(m_state, m_index);
}

bool lua::LuaStackValue::IsCFunction() const
{
	return !!lua_iscfunction(m_state, m_index);
}

bool lua::LuaStackValue::IsNumber() const
{
	return !!lua_isnumber( m_state, m_index );
}

int lua::LuaStackValue::Index() const
{
	return m_index;
}

lua::LuaNumber_t lua::LuaStackValue::GetNumber() const
{
	return lua_tonumber( m_state, m_index );
}

lua::LuaNumber_t lua::LuaStackValue::CheckNumber() const
{
	return luaL_checknumber( m_state, m_index );
}

lua::LuaNumber_t lua::LuaStackValue::OptNumber( LuaNumber_t default ) const
{
	return luaL_optnumber( m_state, m_index, default );
}

int lua::LuaStackValue::ArgError( const char * errMsg )
{
	return luaL_argerror( m_state, m_index, errMsg );
}

const char* lua::LuaStackValue::OptString( const char* default ) const
{
	return luaL_optstring( m_state, m_index, default );
}

bool lua::LuaStackValue::GetBoolean() const
{
	return !!lua_toboolean( m_state, m_index );
}

bool lua::LuaStackValue::IsBoolean() const
{
	return !!lua_isboolean( m_state, m_index );
}

const char* lua::LuaStackValue::GetString() const
{
	return lua_tostring( m_state, m_index );
}

bool lua::LuaStackValue::IsString() const
{
	return lua_type( m_state, m_index ) == LUA_TSTRING;
}

void lua::LuaStackValue::PushValue()
{
	lua_pushvalue( m_state, m_index );
}

void* lua::LuaStackValue::GetUserData() const
{
	return lua_touserdata( m_state, m_index );
}

bool lua::LuaStackValue::IsUserData() const
{
	return !!lua_isuserdata( m_state, m_index );
}

bool lua::LuaStackValue::IsTable() const
{
	return !!lua_istable( m_state, m_index );
}

bool lua::LuaStackValue::IsThread() const
{
	return !!lua_isthread( m_state, m_index );
}

bool lua::LuaStackValue::IsLightUserData() const
{
	return !!lua_islightuserdata( m_state, m_index );
}

void* lua::LuaStackValue::GetLightUserData() const
{
	return lua_touserdata( m_state, m_index );
}

lua_State* lua::LuaStackValue::GetThread() const
{
	return lua_tothread( m_state, m_index );
}

lua::CFunction_t lua::LuaStackValue::GetCFunction() const
{
	return lua_tocfunction( m_state, m_index );
}

lua_State* lua::LuaStackValue::InternalState() const
{
	return m_state;
}

const void* lua::LuaStackValue::ToPointer() const
{
	return lua_topointer( m_state, m_index );
}

bool lua::LuaStackValue::IsEqualByRef( LuaStackValue const& stackValue ) const
{
	return ToPointer() == stackValue.ToPointer();
}

bool lua::LuaStackValue::IsRawEqual( LuaStackValue const& stackValue ) const
{
	return !!lua_rawequal( m_state, m_index, stackValue.m_index );
}

int lua::LuaStackValue::GetInteger() const
{
	return lua_tointeger( m_state, m_index );
}

int lua::LuaStackValue::CheckInteger() const
{
	return luaL_checkinteger( m_state, m_index );
}

int lua::LuaStackValue::OptInteger( int default ) const
{
	return luaL_optinteger( m_state, m_index, default );
}

const char* lua::LuaStackValue::GetUpValue( int n ) const
{
	return lua_getupvalue( m_state, m_index, n );
}

const char* lua::LuaStackValue::SetUpValue( int n )
{
	return lua_setupvalue( m_state, m_index, n );
}

bool lua::LuaStackValue::SetClosureEnv( LuaStackValue& env )
{
	for( int i = 1;; ++i )
	{
		const char* upvalueName = GetUpValue( i );
		if( upvalueName == NULL )
			break;

		bool isEnv = strcmp( upvalueName, LUA_ENV ) == 0;
		lua_pop( m_state, 1 );

		if( isEnv )
		{
			env.PushValue();
			SetUpValue( i );
			return true;
		}

	}

	return false;
}

bool lua::LuaStackValue::PushClosureEnv()
{
	for( int i = 1;; ++i )
	{
		const char* upvalueName = GetUpValue( i );
		if( upvalueName == NULL )
			break;

		bool isEnv = strcmp( upvalueName, LUA_ENV ) == 0;
		if( isEnv )
			return true;

		lua_pop( m_state, 1 );
	}

	return false;
}

lua::LuaStackValue lua::LuaStackValue::PushLength() const
{
	lua_len( m_state, m_index );
	return GetTopValue();
}

size_t lua::LuaStackValue::RawLength() const
{
	return lua_rawlen( m_state, m_index );
}

lua::LuaStackValue lua::LuaStackValue::PushRawGetIndex( int n ) const
{
	lua_rawgeti( m_state, m_index, n );
	return GetTopValue();
}

lua::LuaStackValue lua::LuaStackValue::PushRawGetPointer( const void* ptr ) const
{
	lua_rawgetp( m_state, m_index, ptr );
	return GetTopValue();
}

void lua::LuaStackValue::RawSetIndex( int n )
{
	lua_rawseti( m_state, m_index, n );
}

void lua::LuaStackValue::RawSetPointer( const void* ptr )
{
	lua_rawsetp( m_state, m_index, ptr );
}

lua::LuaStackValue lua::LuaStackValue::GetTopValue() const
{
	return LuaStackValue( m_state, lua_gettop(m_state) );
}


lua::LuaStackTableIterator::LuaStackTableIterator( LuaStackValue const & table )
	: m_table( table )
	, m_hasNext( false )
{
	lua_pushnil( m_table.InternalState() );
	DoNext();
}

lua::LuaStackTableIterator::operator bool() const
{
	return m_hasNext;
}

void lua::LuaStackTableIterator::Next()
{
	CORE_ASSERT( lua_gettop( m_table.InternalState() ) == m_key+1 );
	lua_pop( m_table.InternalState(), 1 ); // pop value
	DoNext();
}

void lua::LuaStackTableIterator::DoNext()
{
	m_hasNext = !!lua_next( m_table.InternalState(), m_table.Index() );
	m_key = lua_gettop( m_table.InternalState() ) - 1;
}

lua::LuaStackValue lua::LuaStackTableIterator::Key() const
{
	return LuaStackValue( m_table.InternalState(), m_key );
}

lua::LuaStackValue lua::LuaStackTableIterator::Value() const
{
	return LuaStackValue( m_table.InternalState(), m_key+1 );
}

