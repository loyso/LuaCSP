//[ operation_cpp_sleep_impl
OpSleep::OpSleep()
	: m_seconds()
{
}

bool OpSleep::Init( lua::LuaStack& args, InitError& initError )
{
	if ( !args[1].IsNumber() )
		return initError.ArgError( 1, "seconds expected" );

	m_seconds = args[1].GetNumber();
	return true;
}

WorkResult::Enum OpSleep::Work( Host&, CspTime_t dt )
{
	m_seconds -= dt;
	return m_seconds > 0 ? WorkResult::YIELD : WorkResult::FINISH;
}
//]

//[ operation_coo_sleep_set_finished
bool OpSleep::Init( lua::LuaStack& args, InitError& initError )
{
	if ( !args[1].IsNumber() )
		return initError.ArgError( 1, "seconds expected" );

	m_seconds = args[1].GetNumber();

	if( m_seconds <= 0 )
		SetFinsihed( true );

	return true;
}
//]


//[ operation_cpp_sleep_reg
int SLEEP( lua_State* luaState )
{
	OpSleep* pSleep = new OpSleep();
	return pSleep->DoInit( luaState );
}

const FunctionRegistration operationDescriptions[] =
{
	  "SLEEP", SLEEP
	, NULL, NULL
};

void RegisterMyOperations( lua::LuaState& state, lua::LuaStackValue& value )
{
	RegisterFunctions( state, value, operationDescriptions );
}

void UnregisterMyOperations( lua::LuaState& state, lua::LuaStackValue& value )
{
	UnregisterFunctions( state, value, operationDescriptions );
}
//]

//[ operation_cpp_push_results
int OpMyDummyOp::PushResults( lua::LuaStack& luaStack )
{
	luaStack.PushString( "hello!" );
	luaStack.PushBoolean( true );
	return 2;
}
//]

//[ operation_cpp_results_example
function bar()
	local str, status = MY_DUMMY_OP()
	log( str, status )
end
//]
