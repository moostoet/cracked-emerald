Route109_SeashoreHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route109_SeashoreHouse_OnTransition
	.byte 0

Route109_SeashoreHouse_OnTransition:
	setflag FLAG_LANDMARK_SEASHORE_HOUSE
	end

Route109_SeashoreHouse_EventScript_Owner::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_SEASHORE_HOUSE, Route109_SeashoreHouse_EventScript_DefeatedTrainers
	goto_if_set FLAG_TEMP_2, Route109_SeashoreHouse_EventScript_AlreadyGaveIntroduction
	msgbox Route109_SeashoreHouse_Text_SeashoreHouseIntro, MSGBOX_DEFAULT
	setflag FLAG_TEMP_2
	release
	end

Route109_SeashoreHouse_EventScript_AlreadyGaveIntroduction::
	msgbox Route109_SeashoreHouse_Text_ShowMeSomeHotMatches, MSGBOX_DEFAULT
	release
	end

Route109_SeashoreHouse_EventScript_DefeatedTrainers::
	specialvar VAR_TEMP_1, GetNumOwnedBadges
	addvar VAR_TEMP_1, 1
	compare VAR_STONES_TAKEN, VAR_TEMP_1
	goto_if_ge Route109_SeashoreHouse_EventScript_DefeatedTrainers_2
	setvar VAR_TEMP_0, ITEM_FIRE_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_WATER_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_THUNDER_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_LEAF_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_MOON_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_SUN_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_SHINY_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_DUSK_STONE
	call EventScript_PushItem
	setvar VAR_TEMP_0, ITEM_DAWN_STONE
	call EventScript_PushItem
	subvar VAR_TEMP_1, VAR_STONES_TAKEN
	buffernumberstring STR_VAR_1, VAR_TEMP_1
	call_if_unset FLAG_TEMP_1, Route109_SeashoreHouse_EventScript_GreatJob
	msgbox Route109_SeashoreHouse_Text_ChooseStone, MSGBOX_DEFAULT
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_SHOW_ITEM
	compare VAR_RESULT, MULTI_B_PRESSED
	goto_if_eq Route109_SeashoreHouse_EventScript_DefeatedTrainers_5
	copyvar VAR_TEMP_1, VAR_RESULT
	giveitem VAR_TEMP_1, 1
	compare VAR_RESULT, FALSE
	goto_if_eq Route109_SeashoreHouse_EventScript_DefeatedTrainers_8
	addvar VAR_STONES_TAKEN, 1
	msgbox Route109_SeashoreHouse_Text_StoneRemaining, MSGBOX_DEFAULT
	setflag FLAG_HIDE_BRAWLY_BLOCK
	release
	end

Route109_SeashoreHouse_EventScript_GreatJob::
	msgbox Route109_SeashoreHouse_Text_GreatJob, MSGBOX_DEFAULT
	setflag FLAG_TEMP_1
	return

EventScript_PushItem::
	bufferitemname STR_VAR_1, VAR_TEMP_0
	dynmultipush EventScript_PushItemIfOwned_Text_0, VAR_TEMP_0
	return

EventScript_PushItemIfOwned_Text_0:
	.string "{STR_VAR_1}$"

Route109_SeashoreHouse_EventScript_DefeatedTrainers_2::
	msgbox Route109_SeashoreHouse_Text_ReturnWithMoreBadges, MSGBOX_DEFAULT
	release
	end

Route109_SeashoreHouse_EventScript_DefeatedTrainers_5::
	msgbox Route109_SeashoreHouse_Text_ComeBackLater, MSGBOX_DEFAULT
	release
	end

Route109_SeashoreHouse_EventScript_DefeatedTrainers_8:
	goto Route109_SeashoreHouse_EventScript_BagFull


Route109_SeashoreHouse_Text_GreatJob:
	.string "Those were some sizzling hot battles!\n"
	.string "You really know how to bring the heat!$"

Route109_SeashoreHouse_Text_ReturnWithMoreBadges:
	.string "Say, I find all sorts of neat stones\n"
	.string "washed up on the shore here.\p"
	.string "Come back when you've got more Badges,\n"
	.string "and I'll share some more with you!$"

Route109_SeashoreHouse_Text_ChooseStone:
	.string "Check out these beautiful stones I\n"
	.string "found while beach combing!\p"
	.string "Pick one you like - I've got {STR_VAR_1}\n"
	.string "more to share with strong TRAINERS!$"

Route109_SeashoreHouse_Text_StoneRemaining:
	.string "The ocean brings in such mysterious\n"
	.string "treasures with the tides.\p"
	.string "Show me more Badges next time, and\n"
	.string "I'll share what else I've found!$"

Route109_SeashoreHouse_Text_ComeBackLater:
	.string "Take your time deciding!\n"
	.string "These stones aren't going anywhere!$"


Route109_SeashoreHouse_EventScript_BagFull::
	msgbox Route109_SeashoreHouse_Text_BagFull, MSGBOX_DEFAULT
	release
	end

Route109_SeashoreHouse_EventScript_NotEnoughSpace::
	msgbox gText_TooBadBagIsFull, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Route109_SeashoreHouse_EventScript_Dwayne::
	trainerbattle_single TRAINER_DWAYNE, Route109_SeashoreHouse_Text_DwayneIntro, Route109_SeashoreHouse_Text_DwayneDefeated, Route109_SeashoreHouse_EventScript_CheckTrainersCompletion
	msgbox Route109_SeashoreHouse_Text_DwaynePostBattle, MSGBOX_AUTOCLOSE
	end

Route109_SeashoreHouse_EventScript_Johanna::
	trainerbattle_single TRAINER_JOHANNA, Route109_SeashoreHouse_Text_JohannaIntro, Route109_SeashoreHouse_Text_JohannaDefeated, Route109_SeashoreHouse_EventScript_CheckTrainersCompletion
	msgbox Route109_SeashoreHouse_Text_JohannaPostBattle, MSGBOX_AUTOCLOSE
	end

Route109_SeashoreHouse_EventScript_Simon::
	trainerbattle_single TRAINER_SIMON, Route109_SeashoreHouse_Text_SimonIntro, Route109_SeashoreHouse_Text_SimonDefeated, Route109_SeashoreHouse_EventScript_CheckTrainersCompletion
	msgbox Route109_SeashoreHouse_Text_SimonPostBattle, MSGBOX_AUTOCLOSE
	end

Route109_SeashoreHouse_EventScript_CheckTrainersCompletion::
	goto_if_not_defeated TRAINER_DWAYNE, Route109_SeashoreHouse_EventScript_TrainersNotCompleted
	goto_if_not_defeated TRAINER_JOHANNA, Route109_SeashoreHouse_EventScript_TrainersNotCompleted
	goto_if_not_defeated TRAINER_SIMON, Route109_SeashoreHouse_EventScript_TrainersNotCompleted
	setflag FLAG_DEFEATED_SEASHORE_HOUSE
	release
	end

Route109_SeashoreHouse_EventScript_TrainersNotCompleted::
	release
	end

Route109_SeashoreHouse_Text_SeashoreHouseIntro:
	.string "I'm the owner of the SEASHORE HOUSE.\n"
	.string "But you can call me MR. SEA!\p"
	.string "What I love above all is to see hot\n"
	.string "POKéMON battles.\p"
	.string "Let me see that your heart burns hot!\p"
	.string "If you can defeat all the TRAINERS\n"
	.string "here, I'll reward your efforts.$"

Route109_SeashoreHouse_Text_ShowMeSomeHotMatches:
	.string "Show me some hot matches!\p"
	.string "I run this SEASHORE HOUSE just for\n"
	.string "that reason alone!$"

Route109_SeashoreHouse_Text_BagFull:
	.string "Oh, but hey, your BAG's jammed full.\n"
	.string "I'll hang on to these for you.$"

Route109_SeashoreHouse_Text_HereYouGo:
	.string "Here you go!$"

Route109_SeashoreHouse_Text_NotEnoughMoney:
	.string "You don't have the money.$"

Route109_SeashoreHouse_Text_ThatsTooBad:
	.string "No?\n"
	.string "That's too bad.$"

Route109_SeashoreHouse_Text_DwayneIntro:
	.string "If you're looking for a battle in the\n"
	.string "SEASHORE HOUSE, you'll find no\l"
	.string "hotter TRAINER than me, matey!$"

Route109_SeashoreHouse_Text_DwayneDefeated:
	.string "That was a hot battle!\n"
	.string "I can accept that loss, matey!$"

Route109_SeashoreHouse_Text_DwaynePostBattle:
	.string "Whenever I'm in SLATEPORT, I enjoy\n"
	.string "hot battles and ice-cold SODA POP!$"

Route109_SeashoreHouse_Text_JohannaIntro:
	.string "Boring battles aren't worth the effort.\p"
	.string "Fiery hot battles are what toughen up\n"
	.string "TRAINERS and POKéMON!$"

Route109_SeashoreHouse_Text_JohannaDefeated:
	.string "That's hot!$"

Route109_SeashoreHouse_Text_JohannaPostBattle:
	.string "Whew, I'm all thirsty.\n"
	.string "Maybe I'll have a SODA POP.$"

Route109_SeashoreHouse_Text_SimonIntro:
	.string "I'm going to show you how great\n"
	.string "my POKéMON are, but don't cry!$"

Route109_SeashoreHouse_Text_SimonDefeated:
	.string "…I lost, but I won't cry…$"

Route109_SeashoreHouse_Text_SimonPostBattle:
	.string "If one of my POKéMON knew the move\n"
	.string "for carrying me across water on its\l"
	.string "back, I could get rid of this inner tube.$"

