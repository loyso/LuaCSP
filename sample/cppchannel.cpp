#include "cppchannel.h"

#include <windows.h>

OpMonitorKeyState::OpMonitorKeyState()
	: m_charToMonitor()
	, m_pressed( false )
	, m_count( 0 )
{
}

OpMonitorKeyState::~OpMonitorKeyState()
{
}

bool OpMonitorKeyState::Init( lua::LuaStack& args, InitError& initError )
{
	if( !args[2].IsString() )
		return initError.ArgError( 2, "character expected." );

	const char* str = args[2].GetString();
	m_charToMonitor = str[0];

	return OpCppChannelOut::Init( args, initError );
}

bool OpMonitorKeyState::IsOutputReady() const
{
	return m_count > 0;
}

csp::WorkResult::Enum OpMonitorKeyState::Update( csp::CspTime_t )
{
	int vk = VkKeyScan( m_charToMonitor ) & 0xFF;
	short state = GetAsyncKeyState( vk );
	bool pressed = !!( state & 0x8000 );
	if( pressed && !m_pressed )
		++m_count;

	m_pressed = pressed;
	return csp::WorkResult::YIELD;
}

csp::ChannelArgument* OpMonitorKeyState::CreateArguments( lua::LuaStack& stack, int& numArguments )
{
	numArguments = 1;
	csp::ChannelArgument* arguments = CORE_NEW csp::ChannelArgument[ numArguments ];

	char str[2] = { m_charToMonitor, 0 };

	for( int i = 0; i < numArguments; ++i )
	{
		stack.PushString( str );
		arguments[ i ].refKey = stack.RefInRegistry();
	}

	--m_count;

	return arguments;
}


int KEYSTATE( lua_State* luaState )
{
	OpMonitorKeyState* pOperation = CORE_NEW OpMonitorKeyState();
	return pOperation->DoInit( luaState );
}

const csp::FunctionRegistration keyStateGlobals[] =
{
	"KEYSTATE", KEYSTATE
	, NULL, NULL
};

void InitializeMyChannels( lua::LuaState& state )
{
	lua::LuaStackValue globals = state.GetStack().PushGlobalTable();
	RegisterFunctions( state, globals, keyStateGlobals );
	state.GetStack().Pop(1);
}

void ShutdownMyChannels( lua::LuaState& state )
{
	lua::LuaStackValue globals = state.GetStack().PushGlobalTable();
	UnregisterFunctions( state, globals, keyStateGlobals );
	state.GetStack().Pop(1);
}
