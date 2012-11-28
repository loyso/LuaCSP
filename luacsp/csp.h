#pragma once

namespace csp
{
    class Host;

	typedef float time_t;

	namespace WorkResult
	{
		enum Enum
		{
			  FINISH = 0
			, YIELD
			, ERROR
		};
	}


    Host& Initialize();
    void Shutdown(Host& host);
}