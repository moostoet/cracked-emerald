GraniteCave_StevensRoom_MapScripts::
	.byte 0

GraniteCave_StevensRoom_EventScript_Steven::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_StevenIntro, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_LETTER
	call Common_EventScript_PlayerHandedOverTheItem
	setflag FLAG_DELIVERED_STEVEN_LETTER
	msgbox GraniteCave_StevensRoom_Text_StevenChallenge, MSGBOX_DEFAULT
	setflag FLAG_BATTLE_FOG
	trainerbattle_no_intro TRAINER_STEVEN_GRANITE_CAVE, GraniteCave_StevensRoom_Text_StevenDefeat
	clearflag FLAG_BATTLE_FOG
	msgbox GraniteCave_StevensRoom_Text_StevenPostBattle, MSGBOX_DEFAULT
	giveitem ITEM_TM_STEEL_WING
	call_if_eq VAR_RESULT, FALSE, GraniteCave_StevensRoom_EventScript_BagFull
	msgbox GraniteCave_StevensRoom_Text_StevenGems, MSGBOX_DEFAULT
	special Special_GiveRandomGems
	playfanfare MUS_LEVEL_UP
	msgbox GraniteCave_StevensRoom_Text_ReceivedGems, MSGBOX_DEFAULT
	waitfanfare
	msgbox GraniteCave_StevensRoom_Text_StevenRegister, MSGBOX_DEFAULT
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox GraniteCave_StevensRoom_Text_RegisteredSteven, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_REGISTERED_STEVEN_POKENAV
	msgbox GraniteCave_StevensRoom_Text_StevenExit, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, GraniteCave_StevensRoom_EventScript_StevenExitNorth
	call_if_eq VAR_FACING, DIR_SOUTH, GraniteCave_StevensRoom_EventScript_StevenExitSouth
	call_if_eq VAR_FACING, DIR_WEST, GraniteCave_StevensRoom_EventScript_StevenExitWestEast
	call_if_eq VAR_FACING, DIR_EAST, GraniteCave_StevensRoom_EventScript_StevenExitWestEast
	playse SE_EXIT
	removeobject LOCALID_GRANITE_CAVE_STEVEN
	release
	end

GraniteCave_StevensRoom_EventScript_StevenExitNorth::
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExit
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_StevenExitWestEast::
	applymovement LOCALID_PLAYER, GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExit
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_StevenExitSouth::
	applymovement LOCALID_PLAYER, GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExitSouth
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_BagFull::
	msgbox GraniteCave_StevensRoom_Text_OhBagIsFull, MSGBOX_DEFAULT
	return

GraniteCave_StevensRoom_Movement_StevenExit:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	step_end

GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit:
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_up
	step_end

GraniteCave_StevensRoom_Movement_StevenExitSouth:
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	delay_8
	step_end

GraniteCave_StevensRoom_Text_StevenIntro:
	.string "You can barely make out a figure\n"
	.string "through the thick fog…\p"
	.string "STEVEN: Hm? A visitor, here?\n"
	.string "Not many can find their way through\l"
	.string "this fog.\p"
	.string "My name is STEVEN. I come here to\n"
	.string "study rare stones.\p"
	.string "Oh? A LETTER for me?$"

GraniteCave_StevensRoom_Text_StevenChallenge:
	.string "STEVEN: I see… From the PRESIDENT.\n"
	.string "Thank you for bringing this.\p"
	.string "And you navigated this cave alone?\n"
	.string "Through all this fog, no less.\p"
	.string "Word is you're challenging the GYMS.\n"
	.string "Is that right?\p"
	.string "… … … … … …\p"
	.string "Then let me see for myself what\n"
	.string "you're made of.\p"
	.string "I don't battle often, but something\n"
	.string "tells me you're worth the trouble.\p"
	.string "Steel yourself. I won't hold back.$"

GraniteCave_StevensRoom_Text_StevenDefeat:
	.string "… Impressive.\n"
	.string "Truly impressive.$"

GraniteCave_StevensRoom_Text_StevenPostBattle:
	.string "STEVEN: Hah… The fog couldn't slow\n"
	.string "you down, and neither could I.\p"
	.string "You have real talent. I haven't\n"
	.string "enjoyed a battle like that in some\l"
	.string "time.\p"
	.string "Here - take this TM. It contains\n"
	.string "STEEL WING, my favorite move.\p"
	.string "I think you'll put it to good use.$"

GraniteCave_StevensRoom_Text_StevenGems:
	.string "STEVEN: Oh, and one more thing.\p"
	.string "I found these gems deeper in the\n"
	.string "cave. I have more than I need.\p"
	.string "They resonate with a POKéMON's\n"
	.string "moves and boost their power once.\l"
	.string "Quite useful in a pinch.$"

GraniteCave_StevensRoom_Text_ReceivedGems:
	.string "{PLAYER} received an assortment\n"
	.string "of type GEMS from STEVEN.$"

GraniteCave_StevensRoom_Text_StevenRegister:
	.string "STEVEN: You know, I have a feeling\n"
	.string "we'll cross paths again.\p"
	.string "Let's register one another in our\n"
	.string "POKéNAVS. I'd like to keep an eye\l"
	.string "on your progress.\p"
	.string "… … … … … …$"

GraniteCave_StevensRoom_Text_RegisteredSteven:
	.string "Registered STEVEN\n"
	.string "in the POKéNAV.$"

GraniteCave_StevensRoom_Text_StevenExit:
	.string "STEVEN: The road ahead only gets\n"
	.string "harder from here. BRAWLY in DEWFORD\l"
	.string "won't go down easily.\p"
	.string "But after what I just saw…\n"
	.string "I think you'll manage just fine.\p"
	.string "Until next time.$"

GraniteCave_StevensRoom_Text_OhBagIsFull:
	.string "Oh, your BAG is full…\n"
	.string "That's too bad, then.$"
