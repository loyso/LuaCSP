
function main()
	log(nil, true, 3.1415, "hello\n")
	test1()
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
			sleep(2)
			log("p2}\n")
		end
	)
	sleep(3)
	log("after\n")
end
