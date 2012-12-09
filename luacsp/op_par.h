#pragma once

#include "operation.h"

namespace csp
{
	class OpPar : public Operation
	{
	public:
		OpPar();
		virtual ~OpPar();

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		bool CheckFinished();

		struct ParClosure
		{
			Process process;
			lua::LuaRef_t refKey;
		};
		ParClosure* m_closures;
		int m_numClosures;

		int m_closureToRun;
	};
}
