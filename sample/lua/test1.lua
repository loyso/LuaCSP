
function main()
	log("main begin\n")
	sleep(0)
	test1()
	log("main end\n")
end

function test1()
	test2()
end

function test2()
	log("before\n")
	par(
		function()
			log("p1{\n")
			sleep(3)
			log("p1}\n")
		end,
		function()
			log("p2{\n")
			par(
				function()
					log("p2a{\n")
					sleep(2)
					log("p2a}\n")
				end,
				function()
					log("p2b{\n")
					sleep(1)
					log("p2b}\n")
				end
			)
			log("p2}\n")
		end
	)
	sleep(3)
	log("after\n")
end
