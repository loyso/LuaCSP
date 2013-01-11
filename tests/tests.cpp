#include <core/core.h>
#include <luacpp/luacpp.h>

#include <luacsp/csp.h>
#include <luacsp/host.h>

#include <luatest/luatest.h>

#include <iostream>
#include <fstream>
#include <string>

namespace TestsResult
{
	enum Enum
	{
		  OK = 0
		, NO_INPUT_FILE = 1
		, OPEN_FILE_ERROR = 2
		, LUA_ERROR = 3
	};
}

int main( int argc, const char* argv[] )
{
	int result = TestsResult::OK;

	if( argc < 2 )
	{
		std::cout << "No input lua file specified" << std::endl;
		return TestsResult::NO_INPUT_FILE;
	}

	core::InitializeCore();
	csp::Host& host = csp::Initialize();
	lua::InitTests( host.LuaState() );

	std::string fileName = argv[1];

	std::ifstream file ( fileName, std::ios::in|std::ios::binary|std::ios::ate );
	if( file.is_open() )
	{
		size_t size = (size_t)file.tellg();

		char* memblock = CORE_NEW char [size];

		file.seekg( 0, std::ios::beg );
		file.read( memblock, size );
		file.close();

		result = TestsResult::LUA_ERROR;
		std:: string chunkname = "@" + fileName;

		lua::Return::Enum loadResult = host.LuaState().LoadFromMemory( memblock, size, chunkname.c_str() );
		if( loadResult == lua::Return::OK )
		{
			lua::Return::Enum chunkCallResult = host.LuaState().Call( 0, 0 );
			if( chunkCallResult == lua::Return::OK )
			{
				const float dt = 1.0f / 30.0f;

				csp::WorkResult::Enum workResult = host.Main();
				while( workResult == lua::Return::YIELD )
					workResult = host.Work( dt );

				result = TestsResult::OK;
			}
		}

		delete[] memblock;
	}
	else
	{
		std::cout << "Can't open input file" << std::endl;
		result = TestsResult::OPEN_FILE_ERROR;
	}

	lua::ShutdownTests( host.LuaState() );
	csp::Shutdown( host );
	core::ShutdownCore();

	return result;
}
