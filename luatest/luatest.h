/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include <luacsp/operation.h>

namespace lua
{
	class LuaState;
	class LuaStackValue;
}

namespace csp
{
	class OpTestSuite_RunAll : public csp::Operation
	{
	public:
		OpTestSuite_RunAll();
		virtual ~OpTestSuite_RunAll();

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual void Terminate( Host& host );

		void InitTestSuite( lua::LuaStack& stack, InitError& initError, const char* suiteName, lua::LuaStackValue& suiteEnv );
		void InitTestSuiteEnv( lua::LuaStack& stack, const char* suiteName, lua::LuaStackValue& suiteEnv, lua::LuaStackValue& table );
		void InitTest( lua::LuaStack& stack, InitError& initError, const char* suiteName, const char* functionName, lua::LuaStackValue& function );

		WorkResult::Enum IsFinished() const;

		struct TestClosure
		{
			TestClosure* pNext;
			Process process;
			lua::LuaRef_t refKey;
			const char* suiteName;
			const char* functionName;
		};

		void UnrefClosure( TestClosure* pClosure );
		void UnrefClosures( TestClosure* pHead );
		static void DeleteClosures( TestClosure*& pHead, TestClosure*& pTail );

		static TestClosure* ListPopFromHead( TestClosure*& pHead, TestClosure*& pTail );
		static void ListAddToTail( TestClosure*& pHead, TestClosure*& pTail, TestClosure& node );

		TestClosure *m_pClosuresHead, *m_pClosuresTail;
		TestClosure* m_pCurrentClosure;
	};

	void InitTests( lua::LuaState& luaState );
	void ShutdownTests( lua::LuaState& luaState );
}
