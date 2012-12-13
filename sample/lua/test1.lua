
function main()
	log("main begin\n")
	testInOut()
	testPar()
	testEmptyPar()
	testEmptyParPar()
	testParMix()
	testAlt1()
	testAlt2()
	testAltTime()
	testTermination()
	testTermination2()
	log("main end\n")
end

function testInOut()
	local ch = Channel()
	PAR(
		function()
			log("send1{\n")
			ch:OUT("hi", 1, true)
			SLEEP(0)
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
			SLEEP(0)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			PAR(
				function()
					log("p2a{\n")
					SLEEP(0)
					log("p2a}\n")
				end,
				function()
					log("p2b{\n")
					SLEEP(0)
					log("p2b}\n")
				end
			)
			log("p2}\n")
		end
	)
	SLEEP(0)
	log("after\n")
end

function testEmptyPar()
	log("before\n")
	PAR(
		function()
			log("p1{\n")
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			log("p2}\n")
		end
	)
	log("after\n")
end

function testEmptyParPar()
	log("before\n")
	PAR(
		function()
			log("p1{\n")
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			PAR(
				function()
					log("p2a{\n")
					log("p2a}\n")
				end,
				function()
					log("p2b{\n")
					log("p2b}\n")
				end
			)
			log("p2}\n")
		end
	)
	log("after\n")
end

function testParMix()
	log("before\n")
	local ch = Channel()
	PAR(
		function()
			log("p1{\n")
			PAR(
				function()
					log("p1a{\n")
					log("p1a}\n")
				end,
				function()
					log("p1b{\n")
					ch:IN()
					log("p1b}\n")
				end
			)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ch:OUT()
			log("p2}\n")
		end,
		function()
			log("p3{}\n")
		end,
		function()
			log("p4{}\n")
		end,
		function()
			log("p5{}\n")
		end,
		function()
			log("p6{}\n")
		end
	)
	log("after\n")
end

function testAlt1()
	log("before\n")
	local ch1 = Channel()
	local ch2 = Channel()
	local ch3 = Channel()
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
				,
				ch3, function( arg1, arg2 )
					log("case3", arg1, arg2, "\n" )
				end
			)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ch3:OUT( "hi", true )
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

function testAltTime()
	log("testAltTime begin\n")
	local ch1 = Channel()

	log("alt polling{\n")
	ALT(
		ch1, function()
			log("polling case ch\n" )
		end
		,
		nil, function()
			log("polling case nil\n" )
		end
	)
	log("alt polling}\n")

	PAR(
		function()
			log("p1{\n")
			ALT(
				time() + 0.8, function()
					log("p1 case 0.8\n" )
				end,
				time() + 0.4, function()
					log("p1 case 0.4\n" )
				end
			)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ALT(
				ch1, function()
					log("p2 case ch\n" )
				end
				,
				time() + 0.1, function()
					log("p2 case 0.1\n" )
				end
			)
			log("p2}\n")
		end
	)
	log("testAltTime end\n")
end

function testTermination()
	PARWHILE(
		function()
			log("a short while{\n")
			SLEEP(0)
			log("a short while}\n")
		end,
		function()
			log("secondary p{\n")
			SLEEP(0)
			log("secondary in progress\n")
			SLEEP(0)
			log("secondary p}\n")
		end,
		function()
			log("ter in\n")
			local ch = Channel()
			ch:IN()
		end,
		function()
			log("ter out\n")
			local ch = Channel()
			ch:OUT()
		end,
		function()
			local ch = Channel()
			log("ter alt\n")
			ALT(
				ch, function()
				end,
				time() + 30, function()
				end
			)
		end,
		function()
			log("ter par\n")
			PAR(
				function()
					local ch = Channel()
					ch:IN()
				end,
				function()
					local ch = Channel()
					ch:OUT()
				end
			)
		end
	)
end

function testTermination2()
	log("testTermination2 begin\n")
	local ch = Channel()
	PARWHILE(
		function()
			log("p1{\n")
			ch:IN()
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			ch:OUT()
			log("p2}\n")
		end
	)
	log("testTermination2 end\n")
end
