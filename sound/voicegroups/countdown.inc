voice_group countdown
	voice_keysplit_all voicegroup_frlg_drumset                        @ GM #1 Drum Kit (percussion channel)
	voice_keysplit voicegroup_piano_keysplit, keysplit_piano                 @ GM #2 Piano keysplit proxy
	.rept 15    @ GM programs 3-18 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_organ2, 255, 0, 255, 127 @ GM #18 Organ
	.rept 15    @ GM programs 20-33 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fingered_bass, 255, 253, 0, 149 @ GM #34 Fingered Bass
	.rept 2     @ GM programs 35-36 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_slap_bass, 255, 235, 128, 99 @ GM #37 Slap Bass
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                          @ GM #38 placeholder
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_synth_bass, 255, 252, 0, 115 @ GM #39 Synth Bass
	.rept 16    @ GM programs 40-55 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_orchestra_hit, 255, 0, 255, 0   @ GM #56 Orchestra Hit
	.rept 8     @ GM programs 57-64 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_square_wave, 255, 0, 255, 0 @ GM #65 Soprano Sax
	.rept 6     @ GM programs 66-71 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_orchestra_cymbal_crash, 255, 246, 0, 216 @ GM #72 Orchestra Cymbal Crash
	.rept 8     @ GM programs 73-80 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_square_1 60, 0, 0, 2, 0, 3, 2, 0 @ GM #81 custom square
	.rept 6     @ GM programs 82-87 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_7, 0, 7, 15, 1 @ GM #88 custom wave
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0	@ GM #89 placeholder
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_15, 0, 7, 15, 0 @ GM #90 custom wave
	.rept 13    @ GM programs 91-103 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0	
	.endr
	voice_square_2 60, 0, 2, 0, 3, 2, 0      @ GM #104 custom square2
	.rept 5     @ GM programs 105-109 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_directsound 60, 0, DirectSoundWaveData_countdown_one, 255, 0, 255, 0    @ GM #110 Bagpipe
	voice_directsound 60, 0, DirectSoundWaveData_countdown_two, 255, 0, 255, 0    @ GM #111 Fiddle
	voice_directsound 60, 0, DirectSoundWaveData_countdown_three, 255, 0, 255, 0  @ GM #112 Shanai
	voice_directsound 60, 0, DirectSoundWaveData_countdown_four, 255, 0, 255, 0   @ GM #113 Tinkle Bell
	voice_directsound 60, 0, DirectSoundWaveData_countdown_five, 255, 0, 255, 0   @ GM #114 Agogo
	.rept 12    @ GM programs 115-126 placeholder
		voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	.endr
	voice_noise_alt 60, 0, 0, 0, 2, 6, 1   @ GM #127 custom noise
	voice_noise_alt 60, 0, 0, 0, 6, 0, 0   @ GM #128 custom noise
