# 1 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 2 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_MapScripts::
# 3 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.byte 0
# 4 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 5 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Blackboard::
# 6 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	lockall
# 7 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_BlackboardListsStatusChanges, MSGBOX_DEFAULT
# 8 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 9 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 10 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 11 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic::
# 12 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	message RustboroCity_PokemonSchool_Text_ReadWhichTopic
# 13 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	waitmessage
# 14 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	multichoicegrid 8, 1, MULTI_STATUS_INFO, 3, FALSE
# 15 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	switch VAR_RESULT
# 16 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 0, RustboroCity_PokemonSchool_EventScript_Poison
# 17 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 1, RustboroCity_PokemonSchool_EventScript_Paralysis
# 18 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 2, RustboroCity_PokemonSchool_EventScript_Sleep
# 19 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 3, RustboroCity_PokemonSchool_EventScript_Burn
# 20 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 4, RustboroCity_PokemonSchool_EventScript_Freeze
# 21 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case 5, RustboroCity_PokemonSchool_EventScript_ExitTopicSelect
# 22 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	case MULTI_B_PRESSED, RustboroCity_PokemonSchool_EventScript_ExitTopicSelect
# 23 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 24 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 25 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Poison::
# 26 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainPoison, MSGBOX_DEFAULT
# 27 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 28 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 29 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 30 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Paralysis::
# 31 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainParalysis, MSGBOX_DEFAULT
# 32 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 33 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 34 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 35 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Sleep::
# 36 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainSleep, MSGBOX_DEFAULT
# 37 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 38 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 39 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 40 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Burn::
# 41 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainBurn, MSGBOX_DEFAULT
# 42 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 43 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 44 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 45 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Freeze::
# 46 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainFreeze, MSGBOX_DEFAULT
# 47 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_ChooseBlackboardTopic
# 48 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 49 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 50 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ExitTopicSelect::
# 51 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	releaseall
# 52 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 53 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 54 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_GameboyKid1::
# 55 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_TradingRightNow, MSGBOX_NPC
# 56 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 57 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 58 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_GameboyKid2::
# 59 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_AlwaysWantedSeedot, MSGBOX_NPC
# 60 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 61 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 62 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_RichBoy::
# 63 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_PokemontCantUseManMadeItems, MSGBOX_NPC
# 64 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 65 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 66 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Lass::
# 67 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ConfusedPokemonAttacksItself, MSGBOX_NPC
# 68 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 69 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 70 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_SchoolKidM::
# 71 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_PokemonHealItselfWithBerry, MSGBOX_NPC
# 72 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 73 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 74 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_StudentNotebook::
# 75 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_StudentsNotes, MSGBOX_SIGN
# 76 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 77 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 78 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_TeacherCheckOnStudentsEast::
# 79 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	applymovement VAR_LAST_TALKED, RustboroCity_PokemonSchool_Movement_TeacherCheckOnStudentsEast
# 80 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	waitmovement 0
# 81 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	return
# 82 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 83 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_TeacherCheckOnStudentsWest::
# 84 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	applymovement VAR_LAST_TALKED, RustboroCity_PokemonSchool_Movement_TeacherCheckOnStudentsWest
# 85 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	waitmovement 0
# 86 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	return
# 87 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 88 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_GaveQuickClaw::
# 89 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ExplainQuickClaw, MSGBOX_DEFAULT
# 90 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	closemessage
# 91 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
# 92 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	waitmovement 0
# 93 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
# 94 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 95 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 96 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Movement_TeacherCheckOnStudentsWest:
# 97 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 98 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 99 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 100 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 101 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_in_place_faster_down
# 102 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 103 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 104 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 105 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 106 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_in_place_faster_left
# 107 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 108 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 109 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 110 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 111 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 112 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_8
# 113 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 114 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 115 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 116 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 117 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 118 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 119 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	step_end
# 120 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 121 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Movement_TeacherCheckOnStudentsEast:
# 122 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 123 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 124 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 125 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 126 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 127 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 128 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_in_place_faster_down
# 129 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 130 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 131 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 132 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_down
# 133 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_in_place_faster_left
# 134 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 135 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 136 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 137 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 138 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_16
# 139 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	delay_8
# 140 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 141 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_right
# 142 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 143 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_up
# 144 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 145 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	walk_left
# 146 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	step_end
# 147 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 148 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_Scott::
# 149 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	lock
# 150 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	faceplayer
# 151 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto_if_set FLAG_MET_SCOTT_AFTER_OBTAINING_STONE_BADGE, RustboroCity_PokemonSchool_EventScript_ScottWatchStudents
# 152 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto_if_set FLAG_MET_SCOTT_RUSTBORO, RustboroCity_PokemonSchool_EventScript_ScottSpokeAlready
# 153 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto_if_set FLAG_BADGE01_GET, RustboroCity_PokemonSchool_EventScript_ScottGreetHasBadge
# 154 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ScottMetAlreadyCut, MSGBOX_DEFAULT
# 155 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	addvar VAR_SCOTT_STATE, 1
# 156 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	setflag FLAG_MET_SCOTT_RUSTBORO
# 157 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
# 158 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 159 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 160 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ScottSpokeAlready::
# 161 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto_if_set FLAG_BADGE01_GET, RustboroCity_PokemonSchool_EventScript_ScottNoticeBadge
# 162 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_StudentTalentLevelUnknown, MSGBOX_DEFAULT
# 163 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
# 164 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 165 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 166 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ScottGreetHasBadge::
# 167 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ScottMetAlreadyStoneBadge, MSGBOX_DEFAULT
# 168 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_MetScottAfterBadge
# 169 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 170 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 171 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ScottNoticeBadge::
# 172 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_ScottStoneBadge, MSGBOX_DEFAULT
# 173 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto RustboroCity_PokemonSchool_EventScript_MetScottAfterBadge
# 174 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 175 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 176 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_MetScottAfterBadge::
# 177 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	addvar VAR_SCOTT_STATE, 1
# 178 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	setflag FLAG_MET_SCOTT_AFTER_OBTAINING_STONE_BADGE
# 179 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
# 180 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 181 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 182 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_EventScript_ScottWatchStudents::
# 183 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_StudentTalentLevelUnknown, MSGBOX_DEFAULT
# 184 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
# 185 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	end
# 186 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 187 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_BlackboardListsStatusChanges:
# 188 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "The blackboard lists status changes\n"
# 189 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "that may affect POKéMON in battle.$"
# 190 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 191 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ReadWhichTopic:
# 192 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Which topic do you want to read?$"
# 193 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 194 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainPoison:
# 195 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If a POKéMON is poisoned, it will\n"
# 196 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "steadily lose HP.\p"
# 197 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "The effects of poison remain after\n"
# 198 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a battle.\p"
# 199 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A poisoned POKéMON's HP will drop\n"
# 200 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "while it is traveling.\p"
# 201 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Heal a poisoning using an ANTIDOTE.$"
# 202 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 203 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainParalysis:
# 204 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If a POKéMON becomes paralyzed,\n"
# 205 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "its SPEED drops.\p"
# 206 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It may also not be able to move while\n"
# 207 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "it is in battle.\p"
# 208 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Paralysis remains after battle.\n"
# 209 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Cure it using a PARLYZ HEAL.$"
# 210 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 211 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainSleep:
# 212 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If a POKéMON falls asleep, it will be\n"
# 213 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "unable to attack.\p"
# 214 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A POKéMON may wake up on its own,\n"
# 215 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "but if a battle ends while it is\l"
# 216 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "sleeping, it will stay asleep.\p"
# 217 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Wake it up using an AWAKENING.$"
# 218 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 219 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainBurn:
# 220 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A burn reduces ATTACK power, and it\n"
# 221 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "steadily reduces the victim's HP.\p"
# 222 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A burn lingers after battle.\n"
# 223 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Cure a burn using a BURN HEAL.$"
# 224 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 225 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainFreeze:
# 226 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If a POKéMON is frozen, it becomes\n"
# 227 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "completely helpless.\p"
# 228 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It will remain frozen after battle.\n"
# 229 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Thaw it out using an ICE HEAL.$"
# 230 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 231 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_EeveeChallenge:
# 232 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "You seem to be a promising TRAINER.\n"
# 233 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I have a special challenge for you.\p"
# 234 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If you can defeat all the TRAINERS\n"
# 235 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "here in the POKéMON SCHOOL, I'll\l"
# 236 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "reward you with an EEVEE.\p"
# 237 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "EEVEE is quite rare and has amazing\n"
# 238 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "potential to evolve in different ways.\p"
# 239 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Show me your battle skills, and the\n"
# 240 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "EEVEE will be yours!$"
# 241 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 242 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_AlreadyHaveEevee:
# 243 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "How is your EEVEE doing? Its evolution\n"
# 244 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "possibilities make it quite special.\p"
# 245 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "With the right items or conditions,\n"
# 246 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "it can evolve into many different forms.\p"
# 247 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I hope you're studying its potential\n"
# 248 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "as thoroughly as my students study\l"
# 249 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "their POKéMON lessons!$"
# 250 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 251 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_DefeatedAllTrainersChallenge:
# 252 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Impressive! You've defeated all the\n"
# 253 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "TRAINERS in my school.\p"
# 254 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "You've shown remarkable skill, but\n"
# 255 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "education isn't complete without\l"
# 256 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a final exam.\p"
# 257 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Let me test your abilities myself\n"
# 258 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "before I hand over that EEVEE.\p"
# 259 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Show me what you've learned about\n"
# 260 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON battles!$"
# 261 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 262 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_GivingEevee:
# 263 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Remarkable! Your battle style shows\n"
# 264 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "both knowledge and instinct.\p"
# 265 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "As promised, here is your EEVEE.\n"
# 266 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's young but full of potential.\p"
# 267 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "EEVEE can evolve into many different\n"
# 268 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "forms depending on how you raise it.\p"
# 269 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I hope this becomes an educational\n"
# 270 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "journey for both of you!$"
# 271 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 272 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ExplainQuickClaw:
# 273 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A POKéMON holding the QUICK CLAW will\n"
# 274 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "occasionally speed up and get to move\l"
# 275 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "before its opponent.\p"
# 276 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "There are many other items that are\n"
# 277 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "meant to be held by POKéMON.\p"
# 278 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Just those alone will give you many\n"
# 279 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "topics to study!$"
# 280 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 281 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_OllieIntro:
# 282 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "I've been studying an interesting move\n"
# 283 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "called TRICK ROOM!\p"
# 284 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "Want to see how it turns the battle\n"
# 285 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "rules upside down?$"
# 286 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 287 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_OllieDefeat:
# 288 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "Even with my space-bending strategy,\n"
# 289 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "you still won! Amazing!$"
# 290 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 291 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_OlliePostBattle:
# 292 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "TRICK ROOM makes slow POKéMON move\n"
# 293 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "first and fast POKéMON move last.\p"
# 294 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "It's like creating a reverse world\n"
# 295 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "where the rules of speed are flipped!\p"
# 296 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "Some POKéMON that people think are\n"
# 297 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "weak become really strong with it.$"
# 298 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 299 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_HarryIntro:
# 300 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "My parents bought me the rarest ICE-type\n"
# 301 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "POKéMON from the coldest regions!\p"
# 302 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "Let me show you how elegant yet\n"
# 303 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "powerful a hail team can be!$"
# 304 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 305 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_HarryDefeat:
# 306 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "Brr... My pristine strategy\n"
# 307 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "melted away...$"
# 308 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 309 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_HarryPostBattle:
# 310 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "I spent all morning polishing my\n"
# 311 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "POKé BALLS with a silk handkerchief...\p"
# 312 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "But I guess that doesn't help much\n"
# 313 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
    .string "in actual battles, does it?$"
# 314 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 315 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_TradingRightNow:
# 316 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I'm trading POKéMON with my friend\n"
# 317 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "right now.$"
# 318 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 319 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_AlwaysWantedSeedot:
# 320 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I always wanted a SEEDOT, and\n"
# 321 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I'm finally getting one!$"
# 322 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 323 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_PokemontCantUseManMadeItems:
# 324 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON can hold items, but they\n"
# 325 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "don't know what to do with man-made\l"
# 326 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "items like POTION and ANTIDOTE.$"
# 327 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 328 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ConfusedPokemonAttacksItself:
# 329 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "You know how some POKéMON moves can\n"
# 330 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "confuse a POKéMON?\p"
# 331 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A confused POKéMON will sometimes\n"
# 332 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "attack itself without meaning to.\p"
# 333 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "But once it leaves battle, it will\n"
# 334 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "return to normal.$"
# 335 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 336 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_PokemonHealItselfWithBerry:
# 337 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A POKéMON holding a BERRY will heal\n"
# 338 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "itself…\p"
# 339 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "There are many kinds of items that\n"
# 340 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON can hold…\p"
# 341 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Boy, it sure is hard taking notes\n"
# 342 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "down…$"
# 343 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 344 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_StudentsNotes:
# 345 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's this student's notebook…\p"
# 346 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON are to be caught using\n"
# 347 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKé BALLS.\p"
# 348 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Up to six POKéMON can accompany\n"
# 349 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a TRAINER.\p"
# 350 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A TRAINER is someone who catches\n"
# 351 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON, raises them, and battles\l"
# 352 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "with them.\p"
# 353 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "A TRAINER's mission is to defeat\n"
# 354 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "the strong TRAINERS who await\l"
# 355 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "challengers in POKéMON GYMS.$"
# 356 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 357 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ScottMetAlreadyCut:
# 358 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Hello? Didn't we meet before?\n"
# 359 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I think back in PETALBURG CITY.\p"
# 360 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Let me introduce myself.\n"
# 361 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "My name's SCOTT.\p"
# 362 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I've been traveling everywhere in\n"
# 363 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "search of outstanding TRAINERS.\p"
# 364 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "More specifically, I'm looking for\n"
# 365 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON battle experts.\p"
# 366 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "So, what brings you to this SCHOOL?\n"
# 367 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Are you a TRAINER, too?\p"
# 368 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "The first thing you should do is to\n"
# 369 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "have a POKéMON learn the move CUT.\p"
# 370 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "If I remember correctly, someone in\n"
# 371 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "this town has CUT.$"
# 372 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 373 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_StudentTalentLevelUnknown:
# 374 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "SCOTT: Hmm…\p"
# 375 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "The talent levels of the students here\n"
# 376 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "are unknown. The potential's there.$"
# 377 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 378 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ScottStoneBadge:
# 379 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "SCOTT: Oh, what's that?\p"
# 380 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's a STONE BADGE, isn't it?\n"
# 381 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "That's pretty impressive, I'd say.\p"
# 382 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "But, you know, I would've loved to see\n"
# 383 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "you in battle.\p"
# 384 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's hard to tell what you're like as\n"
# 385 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a TRAINER from a LEAGUE BADGE.$"
# 386 "data/maps/RustboroCity_PokemonSchool/scripts.pory"

# 387 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
RustboroCity_PokemonSchool_Text_ScottMetAlreadyStoneBadge:
# 388 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Hello? Didn't we meet before?\n"
# 389 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I think back in PETALBURG CITY.\p"
# 390 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Let me introduce myself.\n"
# 391 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "My name's SCOTT.\p"
# 392 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "I've been traveling everywhere in\n"
# 393 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "search of outstanding TRAINERS.\p"
# 394 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "More specifically, I'm looking for\n"
# 395 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "POKéMON battle experts.\p"
# 396 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "Oh, what's that?\n"
# 397 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's a STONE BADGE, isn't it?\p"
# 398 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "That's pretty impressive, I'd say, for\n"
# 399 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a starting TRAINER like yourself.\p"
# 400 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "But, you know, I would've loved to see\n"
# 401 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "you in battle.\p"
# 402 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "It's hard to tell what you're like as\n"
# 403 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	.string "a TRAINER from a LEAGUE BADGE.$"

RustboroCity_PokemonSchool_EventScript_Teacher::
# 407 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	lock
# 408 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	faceplayer
# 409 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	goto_if_set FLAG_RECEIVED_QUICK_CLAW, RustboroCity_PokemonSchool_EventScript_Teacher_2
# 412 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_OlliePostBattle, MSGBOX_DEFAULT
RustboroCity_PokemonSchool_EventScript_Teacher_1:
# 414 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
	end

RustboroCity_PokemonSchool_EventScript_Teacher_2:
# 410 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_OlliePostBattle, MSGBOX_DEFAULT
	goto RustboroCity_PokemonSchool_EventScript_Teacher_1


RustboroCity_PokemonSchool_EventScript_SchoolKid_Ollie::
# 419 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	trainerbattle_single TRAINER_OLLIE_SCHOOL, RustboroCity_PokemonSchool_Text_OllieIntro, RustboroCity_PokemonSchool_Text_OllieDefeat
# 420 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_OlliePostBattle, MSGBOX_DEFAULT
# 421 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
	end


RustboroCity_PokemonSchool_EventScript_RichBoy_Harry::
# 426 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	trainerbattle_single TRAINER_HARRY_SCHOOL, RustboroCity_PokemonSchool_Text_HarryIntro, RustboroCity_PokemonSchool_Text_HarryDefeat
# 427 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	msgbox RustboroCity_PokemonSchool_Text_HarryPostBattle, MSGBOX_DEFAULT
# 428 "data/maps/RustboroCity_PokemonSchool/scripts.pory"
	release
	end

