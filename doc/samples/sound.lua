//[ sound_dialog
function dialog( sound_bank, variant )
	sound_bank.speech1:PLAY()
	sound_bank.speech2:PLAY()
	if variant then
		sound_bank.speech3A:PLAY()
	else
		for i=1,3 do
			sound_bank.speech3B:PLAY()
		end
	end
end
//]

//[ sound_intro
function intro_sound( sound_bank )
	PAR(
		function()
			sound_bank.crowd:PLAY()
		end
		,
		function()
			sound_bank.comment1:PLAY()
			sound_bank.comment2:PLAY()
		end
		,
		function()
			sound_bank.announcement1:PLAY()
			sound_bank.announcement2:PLAY()
		end
	)
end
//]
