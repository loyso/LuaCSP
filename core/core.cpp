#include "core.h"

void core::InitializeCore()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
}

void core::ShutdownCore()
{
}

