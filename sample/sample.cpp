
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

        lua::Return::Value valueLoad = host.LuaState().LoadFromMemory(memblock, size, fileName);
        if( valueLoad == lua::Return::OK )
        {
			lua::Return::Value valueCall = host.LuaState().Call(0, 0);
			if( valueCall == lua::Return::OK )
			{
				int a = 0;
			}
            
			host.Main();
        }

        delete[] memblock;
    }

    csp::Shutdown(host);

	return 0;
}

