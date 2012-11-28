
#include <luacpp/luacpp.h>

#include <luacsp/csp.h>
#include <luacsp/host.h>

#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
    if(argc<2)
    {
        return 1;
    }

    csp::Host& host = csp::Initialize();

    const char* fileName = argv[1];

    std::ifstream file (fileName, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size_t size = (unsigned int)file.tellg();
        
        char* memblock = new char [size];
        
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
				while( mainCall == lua::Return::YIELD )
				{
					host.Work( 0.1f );
				} 
			}
        }

        delete[] memblock;
    }

    csp::Shutdown(host);

	return 0;
}

