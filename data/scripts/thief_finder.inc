@ Thief Finder Key Item Script
@ Uses the same specials as the Thievable Items NPC

EventScript_UseThiefFinder::
	lockall
	special Special_CheckPartyHasCoveOrThief
	goto_if_eq VAR_RESULT, FALSE, EventScript_ThiefFinder_NoMove
	special Special_GetRouteThievableItems
	goto_if_eq VAR_RESULT, 0, EventScript_ThiefFinder_NoItems
	msgbox Text_ThiefFinder_SelectItem, MSGBOX_DEFAULT
	special Special_ShowThievableItemsMenu
	dynmultistack 0, 0, FALSE, 8, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	special Special_HandleThievableItemSelection
	goto_if_eq VAR_RESULT, FALSE, EventScript_ThiefFinder_Cancel
	special Special_ThievableItems_GiveItems
	goto_if_eq VAR_RESULT, FALSE, EventScript_ThiefFinder_BagFull
	bufferitemname STR_VAR_1, VAR_0x8004
	playfanfare MUS_OBTAIN_ITEM
	msgbox Text_ThiefFinder_Received, MSGBOX_DEFAULT
	waitfanfare
	releaseall
	end

EventScript_ThiefFinder_NoMove::
	msgbox Text_ThiefFinder_NeedMove, MSGBOX_DEFAULT
	releaseall
	end

EventScript_ThiefFinder_NoItems::
	msgbox Text_ThiefFinder_NoItems, MSGBOX_DEFAULT
	releaseall
	end

EventScript_ThiefFinder_Cancel::
	releaseall
	end

EventScript_ThiefFinder_BagFull::
	msgbox Text_ThiefFinder_BagFull, MSGBOX_DEFAULT
	releaseall
	end

@ Text Messages
Text_ThiefFinder_NeedMove:
	.string "The THIEF FINDER requires a\n"
	.string "Pokémon that knows COVET or\l"
	.string "THIEF to use.$"

Text_ThiefFinder_NoItems:
	.string "No wild Pokémon have been\n"
	.string "encountered in this area yet.$"

Text_ThiefFinder_SelectItem:
	.string "The THIEF FINDER detected these\n"
	.string "items from wild Pokémon here.\p"
	.string "Which item would you like?$"

Text_ThiefFinder_Received:
	.string "{PLAYER} obtained infinite\n"
	.string "{STR_VAR_1}!$"

Text_ThiefFinder_BagFull:
	.string "Your Bag is full!$"
