
//[ seq_foo
function foo( ch1, ch2 )
	local d = 10
	ch1:OUT( d, "hello" )
	SLEEP(10)
	local v1, v2 = ch2:IN()
end
//]

//[ seq_sleeps
function bar()
	SLEEP(10)
	SLEEP(20)
	for i = 1,10 do
		SLEEP(1)
	end
end
//]
