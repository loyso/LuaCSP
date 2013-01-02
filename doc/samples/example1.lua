//[ example1_foobar
function foo( ch )
	ch:OUT( 42 )
end

function bar( ch )
	local v = ch:IN()
end
//]

//[ example1_main
function main()
end
//]

//[ example1_construct_channel
function main()
	local ch = Channel:new()
end
//]


