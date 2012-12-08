
function main()
	log("main begin\n")
	-- testInOut()
	-- testPar()
	testAlt1()
	-- testAlt2()
	log("main end\n")
end

function testInOut()
	local ch = Channel()
	PAR(
		function()
			log("send1{\n")
			ch:OUT("hi", 1, true)
			SLEEP(3)
			ch:OUT()
			log("send1}\n")
		end,
		function()
			log("rcv1{\n")
			local str, num, bool = ch:IN()
			log("received:", str, num, bool, "\n")
			ch:IN()
			log("rcv1}\n")
		end
	)
end

function testPar()
	log("before\n")
	PAR(
		function()
			log("p1{\n")
			SLEEP(3)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			PAR(
				function()
					log("p2a{\n")
					SLEEP(2)
					log("p2a}\n")
				end,
				function()
					log("p2b{\n")
					SLEEP(1)
					log("p2b}\n")
				end
			)
			log("p2}\n")
		end
	)
	SLEEP(3)
	log("after\n")
end

function testAlt1()
	log("before\n")
	local ch1 = Channel()
	local ch2 = Channel()
	PAR(
		function()
			log("p1{\n")
			ALT(
				ch1, function( arg1, arg2 )
					log("case1", arg1, arg2, "\n" )
				end
				,
				ch2, function( arg1, arg2 )
					log("case2", arg1, arg2, "\n" )
				end
			)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ch1:OUT( "hi", true )
			log("p2}\n")
		end
	)
	log("after\n")
end

function testAlt2()
	log("before\n")
	local ch1 = Channel()
	local ch2 = Channel()
	PAR(
		function()
			log("p1{\n")
			ch2:OUT( "hi", true )
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ALT(
				ch1, function( arg1, arg2 )
					log("case1", arg1, arg2, "\n" )
				end
				,
				ch2, function( arg1, arg2 )
					log("case2", arg1, arg2, "\n" )
				end
			)
			log("p2}\n")
		end
	)
	log("after\n")
end
