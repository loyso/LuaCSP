/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "luacpp.h"

namespace lua
{
	class LuaStackValue
	{
	public:
		LuaStackValue();
		LuaStackValue( lua_State* state, int index);

		bool IsNil() const;
		bool IsFunction() const;
		bool IsCFunction() const;
		bool IsNumber() const;
		bool IsString() const;
		bool IsBoolean() const;
		bool IsUserData() const;
		bool IsLightUserData() const;
		bool IsTable() const;
		bool IsThread() const;

		LuaStackValue PushLength() const;
		size_t RawLength() const;

		LuaStackValue PushRawGetIndex( int n ) const;
		LuaStackValue PushRawGetPointer( const void* ptr ) const;

		void RawSetIndex( int n );
		void RawSetPointer( const void* ptr );

		LuaNumber_t GetNumber() const;
		LuaNumber_t CheckNumber() const;
		LuaNumber_t OptNumber( LuaNumber_t default ) const;
		
		int GetInteger() const;
		int CheckInteger() const;
		int OptInteger( int default ) const;

		const char* GetString() const; 
		const char* CheckString() const; 
		const char* OptString( const char* default ) const; 

		bool GetBoolean() const;
		void* GetUserData() const;
		void* GetLightUserData() const;
		lua_State* GetThread() const;
		CFunction_t GetCFunction() const;

		int ArgError( const char* errMsg );

		int Index() const;
		void PushValue();

		const void* ToPointer() const;
		
		bool IsEqualByRef( LuaStackValue const& stackValue ) const;
		bool IsRawEqual( LuaStackValue const& stackValue ) const;

		const char* GetUpValue( int n ) const;
		const char* SetUpValue( int n );
		bool SetClosureEnv( LuaStackValue& env );
		bool PushClosureEnv();

		lua_State* InternalState() const;

	private:
		LuaStackValue GetTopValue() const;

		lua_State* m_state;
		int m_index;
	};

	class LuaStackTableIterator
	{
	public:
		LuaStackTableIterator( LuaStackValue const & table );

		operator bool() const;
		void Next();

		LuaStackValue Key() const;
		LuaStackValue Value() const;

	private:
		void DoNext();

		LuaStackValue m_table;
		int m_key;
		bool m_hasNext;
	};
}
