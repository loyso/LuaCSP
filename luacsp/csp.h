#pragma once

namespace csp
{
    class Host;

	namespace WorkResult
	{
		enum Enum
		{
			  RESUME = 0
			, YIELD			
			, ERROR_REPORTED
		};
	}

    Host& Initialize();
    void Shutdown(Host& host);
}