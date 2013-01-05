//[ parwhile_syntax
PARWHILE(
	function()
		--- main child process
	end
	,
	function()
		--- child process1 (potential termination victim)
	end
	,
...
	,
	function()
		--- child processN (potential termination victim)
	end
)
//]

//[ parwhile_bar 
function bar()
	SLEEP(3)
	PARWHILE(
		function()
			SLEEP(1) --- main process 
		end
		,
		function()
			SLEEP(3)
		end
		,
		function()
			SLEEP(4)
		end
	)
	SLEEP(5) 
end
//]

//[ parwhile_noreturn
function foo( ch )
	PARWHILE(
		function()
			ch:IN()
		end
		,
		function()
			while true do
				...
				SLEEP(0)
			end
		end
	)
end
//]

//[ parwhile_simple_cleanup
function foo( ch )
	PARWHILE(
		function()
			ch:IN()
			--- put clean up code here!!!
		end
		,
		function()
			while true do
				...
				SLEEP(0)
			end
		end
	)
end
//]
