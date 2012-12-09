#include "luastackvalue.h"

extern "C"
{
#include <lua-5.2.1/src/lua.h>
#include <lua-5.2.1/src/lauxlib.h>
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

bool lua::LuaStackValue::IsLightUserData() const
{
	return !!lua_islightuserdata( m_state, m_index );
}

void* lua::LuaStackValue::GetLightUserData() const
{
	return lua_touserdata( m_state, m_index );
}
