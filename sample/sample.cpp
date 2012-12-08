#include <core/core.h>
#include <luacpp/luacpp.h>

#include <luacsp/csp.h>
#include <luacsp/host.h>

#include <iostream>
#include <fstream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main(int argc, const char* argv[])
{
    if(argc<2)
    {
        return 1;
    }

	core::InitializeCore();

    csp::Host& host = csp::Initialize();

    const char* fileName = argv[1];

    std::ifstream file (fileName, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size_t size = (unsigned int)file.tellg();
        
        char* memblock = CORE_NEW char [size];
        
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();

		char chunkname [1024];
		sprintf( chunkname, "@%s", fileName );

        lua::Return::Enum valueLoad = host.LuaState().LoadFromMemory(memblock, size, chunkname);
        if( valueLoad == lua::Return::OK )
        {
			lua::Return::Enum valueCall = host.LuaState().Call(0, 0);
			if( valueCall == lua::Return::OK )
			{
				csp::WorkResult::Enum mainCall = host.Main();
				const float dt = 0.01f;
				while( mainCall == lua::Return::YIELD )
				{
					Sleep( (unsigned int)(dt * 1000.0) );
					mainCall = host.Work( dt );
				} 
			}
        }

        delete[] memblock;
    }

    csp::Shutdown(host);
	core::ShutdownCore();

	std::cout << std::endl << "Press Enter to continue...";
	std::string input;
	std::getline( std::cin, input );

	return 0;
}

