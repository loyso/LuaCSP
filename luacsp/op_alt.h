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
	class OpAlt : public Operation, ChannelAttachmentIn_i
	{
	public:
		OpAlt();
		virtual ~OpAlt();

	private:
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual void Terminate( Host& host );

		virtual bool Init( lua::LuaStack& args, InitError& initError );
		bool CheckArgs( lua::LuaStack& args, InitError& initError ) const;
		void InitCases( lua::LuaStack& args );

		void DetachChannels() const;
		void UnrefChannels( lua::LuaStack const& stack );
		void UnrefArguments( lua::LuaStack const& stack );
		void UnrefClosures( lua::LuaStack const& stack );
		void UnrefProcess( lua::LuaStack const& stack );

		WorkResult::Enum StartTriggeredProcess( Host& host );
		bool SelectChannelProcessToTrigger( Host& host );
		void SelectTimeProcessToTrigger( Host& host );

		virtual void MoveChannelArguments( Channel& channel, ChannelArgument* arguments, int numArguments );
		virtual Process& ProcessToEvaluate();
		virtual void CloseChannel( csp::Host & host, Channel& channel );

		virtual void DebugCheck( Host& host ) const;

		struct AltCase
		{
			AltCase();

			lua::LuaRef_t m_closureRefKey;

			Channel* m_pChannel;
			lua::LuaRef_t m_channelRefKey;
			CspTime_t m_time;				
		};
		AltCase* m_cases;
		int m_numCases;

		AltCase* FindCaseForChannel( Channel& channel ) const;
		void CloseCase( lua::LuaStack& stack, AltCase& altCase );

		AltCase* m_pCaseTriggered;
		AltCase* m_pNilCase;

		Process m_process;
		lua::LuaRef_t m_processRefKey;

		ChannelArgument* m_arguments;
		int m_numArguments;
		bool m_argumentsMoved;
	};
}