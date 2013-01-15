/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "operation.h"

namespace csp
{
	class OpPar : public Operation
	{
	public:
		OpPar();
		virtual ~OpPar();

	protected:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		bool CheckFinished();
		void UnrefClosures();

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual void Terminate( Host& host );

		virtual void DebugCheck( Host& host ) const;

		struct ParClosure
		{
			Process process;
			lua::LuaRef_t refKey;
		};
		ParClosure* m_closures;
		int m_numClosures;

		int m_closureToRun;
	};

	class OpParWhile : public OpPar
	{
	public:
		virtual WorkResult::Enum Evaluate( Host& host );
	
	private:
		void DoTerminate( Host& host );
	};
}
