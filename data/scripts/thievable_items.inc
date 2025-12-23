@ NPC that gives the Thief Finder key item
@ The actual functionality is now in the key item (see thief_finder.inc)

EventScript_ThievableItemsGuide::
	lock
	faceplayer
	goto_if_set FLAG_OBTAINED_THIEF_FINDER, EventScript_ThievableItemsGuide_AlreadyHave
	msgbox Text_ThievableItemsGuide_Intro, MSGBOX_DEFAULT
	giveitem ITEM_THIEF_FINDER
	setflag FLAG_OBTAINED_THIEF_FINDER
	msgbox Text_ThievableItemsGuide_Explain, MSGBOX_DEFAULT
	release
	end

EventScript_ThievableItemsGuide_AlreadyHave::
	msgbox Text_ThievableItemsGuide_Reminder, MSGBOX_DEFAULT
	release
	end

Text_ThievableItemsGuide_Intro:
	.string "Hello there, trainer!\p"
	.string "I'm an expert on wild Pokémon\n"
	.string "behavior and their held items!\p"
	.string "I've invented a device that can\n"
	.string "detect what items wild Pokémon\l"
	.string "carry in an area.\p"
	.string "Here, take this!$"

Text_ThievableItemsGuide_Explain:
	.string "That's a THIEF FINDER!\p"
	.string "Use it anywhere to see what items\n"
	.string "wild Pokémon carry in that area.\p"
	.string "You'll need a Pokémon that knows\n"
	.string "COVET or THIEF to use it, and\l"
	.string "you must have encountered at\l"
	.string "least one wild Pokémon there.$"

Text_ThievableItemsGuide_Reminder:
	.string "How's that THIEF FINDER working\n"
	.string "out for you?\p"
	.string "Remember, use it to find items\n"
	.string "that wild Pokémon carry!$"
