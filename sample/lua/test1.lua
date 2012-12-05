
function main()
	log("main begin\n")
	SLEEP(0)
	test1()
	test3()
	log("main end\n")
end

function test1()
	test2()
end

function test2()
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

function test3()
	local ch = Channel()
	PAR(
		function()
			log("send1{\n")
			ch:OUT("hi", 1, true)
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