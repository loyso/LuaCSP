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

TestsResult::Enum LoadLuaFile( csp::Host& host, const std::string& fileName )
{
	std::ifstream file ( fileName, std::ios::in|std::ios::binary|std::ios::ate );
	if( !file.is_open() )
	{
		std::cout << "Can't open input file" << std::endl;
		return TestsResult::OPEN_FILE_ERROR;
	}

	TestsResult::Enum result = TestsResult::LUA_ERROR;

	size_t size = (size_t)file.tellg();

	char* memblock = CORE_NEW char [size];

	file.seekg( 0, std::ios::beg );
	file.read( memblock, size );
	file.close();

	std::string chunkname = "@" + fileName;

	lua::Return::Enum loadResult = host.LuaState().LoadFromMemory( memblock, size, chunkname.c_str() );
	if( loadResult == lua::Return::OK )
	{
		lua::Return::Enum chunkCallResult = host.LuaState().Call( 0, 0 );
		if( chunkCallResult == lua::Return::OK )
			result = TestsResult::OK;
	}

	delete[] memblock;

	return result;
}

TestsResult::Enum EvaluateLuaMain( csp::Host& host )
{
	TestsResult::Enum result = TestsResult::LUA_ERROR;

	const float dt = 1.0f / 60.0f;

	csp::WorkResult::Enum workResult = host.Main();
	while( workResult == lua::Return::YIELD )
		workResult = host.Work( dt );

	result = TestsResult::OK;

	return result;
}

int main( int argc, const char* argv[] )
{
	TestsResult::Enum result = TestsResult::OK;

	if( argc < 2 )
	{
		std::cout << "No input lua files specified" << std::endl;
		return TestsResult::NO_INPUT_FILE;
	}

	core::InitializeCore();
	csp::Host& host = csp::Initialize();
	csp::InitTests( host.LuaState() );

	for( int i = 1; i < argc; ++i )
	{
		std::string fileName = argv[i];
		TestsResult::Enum loadResult = LoadLuaFile( host, fileName );
		if( loadResult != TestsResult::OK )
			result = loadResult;
	}

	if( result == TestsResult::OK )
	{
		result = EvaluateLuaMain( host );
	}

	csp::ShutdownTests( host.LuaState() );
	csp::Shutdown( host );
	core::ShutdownCore();

	return result;
}
