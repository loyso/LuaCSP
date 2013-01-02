//[ alt_syntax
ALT(
	guard1, function()
		--- process 1
	end
	,
	guard2, function()
		--- process 2
	end
	,
...
	,
	guardN, function()
		--- process N
	end
)
//]

//[ alt_channel_guards
function foo( ch1, ch2, ch3, ch4 )
	ALT(
		ch1, function( arg1, arg2 )
			ch3:OUT( arg1, arg2 )
		end
		,
		ch2, function()
			ch4:OUT()
		end
	)
end
//]

//[ alt_time
function foo( ch )
	ALT(
		time() + 100, function()
			--- timeout of 100 seconds expired!
		end
		,
		ch, function()
			local v = ch:IN()
		end
	)
end
//]

//[ alt_nil
function foo( ch1, ch2 )
	ALT(
		ch1, function()
			local v = ch1:IN()
			ch2:OUT( v )
		end
		,
		nil, function()
			local zero = 0
			ch2:OUT( zero )
		end
	)
end
//]
