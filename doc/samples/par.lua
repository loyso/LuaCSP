
//[ par_foo
function foo( ch1, ch2 )
	PAR(
		function()
			local x = 3
			ch1:OUT( x )
		end
		,
		function()
			local y = ch2:IN()
		end
		,
		function()
			SLEEP(20)
		end
	)
end
//]

//[ par_bar
function bar()
	SLEEP(3)
	PAR(
		function()
			SLEEP(1)
		end
		,
		function()
			SLEEP(2)
		end
		,
		function()
			SLEEP(3)
		end
	)
	SLEEP(4)
end
//]


//[ par_sharedchannel
function main()
	local ch = Channel:new() --- shared!
	PAR(
		function()
			local a = 1 --- isolated, hidden
			ch:OUT( a )
		end
		,
		function()
			local b = ch:IN()
		end
	)
end
//]
