/*
 Ultima 7 usecode dump/disassembly utility
 Distributed under GPL

 Note:
	- the source code comments below are possibly SPOILERS. You have beed warned.

 Maintainer:
 	Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
 
 History:
 - originally written June 99 by Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
 Thanks to Keldon Jones (keldon@umr.edu)
	and Wouter Dijklslag aka Wody Dragon (wody@wody.demon.nl) for their help
 11-Oct-99
	- added "function search by opcode & intrinsic" feature
	- added "unknown opcode & intrinsic counting" feature
 12-Oct-99
	- the good deal of intrinsic functions is now known
 6-May-00
	- the current version
 
 The source must be buildable on any C compiler and runnable on any OS
	(tested on Win32 and Linux).
 See source file comments for the description of usecode opcodes & intrinsic functions
	(the latter one differs between BG & SI)
 
 Some general usecode ideas:
 - usecode functions 0x0-0x3ff are shape handlers - called on double-clicks & other
	event with appropriate shapes
 - usecode functions 0x401-0x4ff are NPC handlers - called on double-clicks & other
	event with appropriate NPCs - NPCID + 0x400 (0x401 for Iolo, 0x417 for LB in BG,
	0x41f for Rotoluncia in SI etc).
 - usecode functions 0x500-0x5ff is for Wisps & guards (nonNPC characters able to talk)
	(these ranges seems to be hardcoded)
 - stack machine used to execute bytecodes
 - the machine's state is:
	stack
	local variables(forgotten on function exit, first N of them are call arguments -
					first pushed is 0, next are 1, 2...)
	game flags
	ItemRef (???seems to be valid only for top-level functions-event handlers
				or maybe is persistent till quitting usecode executuion???)
	EventID (???seems to be valid only for top-level functions-event handlers
				or maybe is persistent till quitting usecode executuion???)
 - game flags are bytes treated as booleans (0/1), persistent across engine shutdown/restart
	and stored as a simple array (??? 0 or 1 based. Don't remember. Flag 3 means
	- Tetrahedron is down, flag 4 means - Sphere is down) in GAMEDAT\FLAGINIT.
 - usecode can also manipulate items & NPCs by means of intrinsic functions
 - "add" opcode can sum strings (concatenation). Also it can add integer to string
 - any array operations can be peformed on scalar values. In fact, each scalar value is
	treated by the array operations as an array with a single element. Vice versa is also
	true - for instance, ItemsNearItem() function returns an array. Sometimes it is used in
	enum/next loop as an array, but sometimes it is used as an itemref.
 - array indices are 1-based as in VB
 - itemref is a unique ID of the given item. For NPCs, itemref is (-NPCID). For other items,
	itemrefs seems to be non-persistent (not saved to savegame & re-invented on each
	engine startup???) indexes into engine's item lists
 - there is a value called "referent" which identifies item & stored in U7IBUF
    Maybe Itemref is the same thing?
 - -356 is always an Itemref for Avatar. So, Avatar's NPC ID is possibly 356.
 - usecode execution starts from the event handler function. It is called by the engine
	without arguments (double-click) in some cases. ItemRef & EventID are set
	before entering usecode.
 - the easiest case is double-click on some item. In this case, a usecode event handler
	function called. Function ID usually matches the shape's Type number
	or is (NPCID + 0x400) for NPCs.
	ItemRef is set to the item double-clicked, EventID is set to 1 (double-click).
 - other causes for the engine to call usecode function:
	- events scheduled by intrinsic functions 1 & 2 have EventID 2
	- item is put on another item - the underlying item's handler is called with EventID 3
		(Penumbra's plaque)
	- usecode Egg - also calls a function with EventID 3
	- use item as a weapon (flammable oil) - EventID 4
	- EventID 5 and 6 - placing the item somewhere in the NPC's inventory
		(ring of invisibility)
	- NPC being beaten to death (examples: Hook in BG, Dracothaxus in FoV,
					Pomdirgun & Rotoluncia in SI) - 7 in SI???
	- Avatar & NPC approaching to some distance??? - 9 in SI
 - hex coords to sextant coords formula (same in BG and SI )
	( x - 933 ) / 10, ( y - 1134 ) / 10
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 Opcode flags
 Just a 16bit word
*/
#define IMMED					1
/* Immediate 1 byte operand */
#define IMMED_BYTE				2
/* Will print a part of data string as comment */
#define	DATA_STRING				4
/* Will add command offset before printing */
#define	RELATIVE_JUMP				8
/* Will print third byte as decimal after comma */
#define CALL					16
/* Will print in square brackets */
#define VARREF					32
/* Will print in square brackets with "flag:" prefix */
#define FLGREF					64
/* Call of usecode function using extern table */
#define EXTCALL					128

/* Opcode descriptor */
typedef struct _opcode_desc
{
	/* Mnemonic - NULL if not known yet */
	const char* mnemonic;
	/* Number of operand bytes */
	int nbytes;
	/* Type flags */
	unsigned char type;
} opcode_desc;

/* Opcode table - common to BG & SI */
static opcode_desc opcode_table[] =
{
	{ NULL, 0, 0 },						/* 00 */
	{ NULL, 0, 0 },						/* 01 */
	/* Next iteration of For Each-style loop
		Operands are 1,2, <current>, <array variable>, <jump>
		<current> is set to next value from <array variable>
		Jumps to <jump> if array ended
		??? what are 1,2? Are they used?
	*/
	{ "next", 10, VARREF | RELATIVE_JUMP },			/* 02 */
	{ NULL, 0, 0 },						/* 03 */
	/* Asks user to select one the talk answers
		Jumps where specified if no answer available
	*/
	{ "ask", 2, RELATIVE_JUMP },				/* 04 */
	/* Pops a value from the top of stack, jump if false, zero or empty array */
	{ "jne", 2, RELATIVE_JUMP },				/* 05 */
	/* jump */
	{ "jmp", 2, RELATIVE_JUMP },				/* 06 */
	/* Pops the top-of-stack string & jumps if the string is NOT the last talk answer 
		??? first operand seems to be always 1
	*/
	{ "jmpa", 4, IMMED | RELATIVE_JUMP},			/* 07 */
	{ NULL, 0, 0 },						/* 08 */
	/* Adds two values on the stack, popping them & pushing result 
		Can be used to add integer to string - in this case integer is converted to string 
		and 2 strings are concatenated */
	{ "add", 0, 0 },					/* 09 */
	/* Decrements second value of the stack by first value, popping them & pushing result */
	{ "sub", 0, 0 },					/* 0a */
	/* Divides second value of the stack by first value, popping them & pushing result */
	{ "div", 0, 0 },					/* 0b */
	/* Multiplies two values on the stack, popping them & pushing result */
	{ "mul", 0, 0 },					/* 0c */
	/* Divides second value of the stack by first value, popping them & pushing reminder */
	{ "mod", 0, 0 },					/* 0d */
	/* Boolean AND on two values on stack, popping them & pushing result
		Top-of-stack variable is the left one in case of string addition */
	{ "and", 0, 0 },					/* 0e */
	/* Boolean OR on two values on stack, popping them & pushing result */
	{ "or", 0, 0 },						/* 0f */
	/* Inverts a boolean value on top of stack */
	{ "not", 0, 0 },					/* 10 */
	{ NULL, 0, 0 },						/* 11 */
	/* Pops a stack value to given local variable */
	{ "pop", 2, VARREF },					/* 12 */
	/* Pushes a TRUE boolean value on the stack */
	{ "push\ttrue", 0, 0 },					/* 13 */
	/* Pushes a FALSE boolean value on the stack */
	{ "push\tfalse", 0, 0 },				/* 14 */
	{ NULL, 0, 0 },						/* 15 */
	/* Pops 2 values from the stack, pushes boolean value - 
		TRUE if (top-of-stack-1) value "?" then top-of-stack
		(where "?" can be greater, greater or equal etc...)
	*/
	{ "cmpgt", 0, 0 },					/* 16 */
	{ "cmplt", 0, 0 },					/* 17 */
	{ "cmpge", 0, 0 },					/* 18 */
	{ "cmple", 0, 0 },					/* 19 */
	{ "cmpne", 0, 0 },					/* 1a */
	{ NULL, 0, 0 },						/* 1b */
	/* Adds a string from data segment to string register current contents */
	{ "addsi", 2, DATA_STRING },				/* 1c */
	/* Pushes a string value given by 16bit string offset in data segment to stack */
	{ "pushs", 2, DATA_STRING },				/* 1d */
	/* Pops specified number of values from stack, builds an array from them
		& pushes it on the stack 
		Pushes the empty array to top of the stack if operand is 0
	*/
	{ "arrc", 2, IMMED },					/* 1e */
	/* Pushes immediate 16bit integer to stack */
	{ "pushi", 2, IMMED },					/* 1f */
	{ NULL, 0, 0 },						/* 20 */
	/* Pushes a local variable on stack */
	{ "push", 2, VARREF },					/* 21 */
	/* Compares 2 values on the stack, pops them & pushes TRUE if they are equal */
	{ "cmpeq", 0, 0 },					/* 22 */
	{ NULL, 0, 0 },						/* 23 */
	/* Calls a usecode function - function number is 0-based index to externs array */
	{ "call", 2, VARREF | EXTCALL },			/* 24 */
	/* Return from function without result returned on the stack */
	{ "ret", 0, 0 },					/* 25 */
	/* Uses the top-of-stack value to index (1-based) the array variable, 
		pops index & pushes result - the value from the array */
	{ "aget", 2, VARREF },					/* 26 */
	{ NULL, 0, 0 },						/* 27 */
	{ NULL, 0, 0 },						/* 28 */
	{ NULL, 0, 0 },						/* 29 */
	{ NULL, 0, 0 },						/* 2a */
	{ NULL, 0, 0 },						/* 2b */
	/* ??? Looks like to be the same as "exit"
		??? Suggestion: maybe "exit" is for functions, while "exit2" is for event handlers? */
	{ "exit2", 0, 0 },					/* 2c */
	/* Pop the top-of-stack value & sets it as a return value */
	{ "popr", 0, 0 },					/* 2d */
	/* Opens a new For Each-style enumeration loop.
		Always followed by "next" opcode??? */
	{ "enum", 0, 0 },					/* 2e */
	/* Adds local variable's string value to string register current contents */
	{ "addsv", 2, VARREF },					/* 2f */
	/* If (top-of-stack - 1) value is in top-of-stack array, pushes true, 
		otherwise pushes false - after popping array & value */
	{ "in", 0, 0 },						/* 30 */
	/* Something strange with 2 varrefs (???) as operands
		Appears in BG only - in talk to Raymundo in the Theatre - trying to sing a song
		I -am- the Avatar
	*/
	{ "???", 4, 0 },					/* 31 */
	/* Return with a result returned on the stack */
	{ "retr", 0, 0 },					/* 32 */
	/* Displays the string register value (to current talk, sign, scroll or book),
		string register emptied */
	{ "say", 0, 0 },					/* 33 */
	{ NULL, 0, 0 },						/* 34 */
	{ NULL, 0, 0 },						/* 35 */
	{ NULL, 0, 0 },						/* 36 */
	{ NULL, 0, 0 },						/* 37 */
	/* Calls engine's intrinsic function with specified number of parameters 
	 	(popping them). The return value remains on stack */
	{ "callis", 3, CALL },					/* 38 */
	/* Calls engine's intrinsic function with specified number of parameters 
	 	(popping them). No return value  */
	{ "calli", 3, CALL },					/* 39 */
	{ NULL, 0, 0 },						/* 3a */
	{ NULL, 0, 0 },						/* 3b */
	{ NULL, 0, 0 },						/* 3c */
	{ NULL, 0, 0 },						/* 3d */
	/* Pushes identifier of the item ( for which the usecode event handler is called ) 
		on the stack */
	{ "push\titemref", 0, 0 },				/* 3e */
	/* Aborts the function & all usecode execution, returning to the engine */
	{ "exit", 0, 0 },					/* 3f */
	/* Removes all answers from the current talk */
	{ "cla", 0, 0 },					/* 40 */
	{ NULL, 0, 0 },						/* 41 */
	/* Pushes game flag's value (boolean) on the stack */
	{ "pushf", 2, FLGREF },					/* 42 */
	/* Pops the stack value to the game flag (boolean) */
	{ "popf", 2, FLGREF },					/* 43 */
	/* Pushes an immediate byte to the stack */
	{ "pushbi", 1, IMMED_BYTE },				/* 44 */
	{ NULL, 0, 0 },						/* 45 */
	/* Uses the top-of-stack value to index (1-based) the array variable, (top-of-stack - 1)  as
		the new value, and updates a value in the array slot (local variable specified
		in the operand) */
	{ "aput", 2, VARREF },					/* 46 */
	/* Call of usecode function - function # is the operand
		???Suggestion: functions are divided into event handlers (called by external code
		and the usecode) and functions (called by usecode only). "calle" is used to
		call the event handler from the usecode, while "call" is used to call a function
	*/
	{ "calle", 2, EXTCALL },				/* 47 */
	/* Pushes the cause of usecode event handler call on the stack */
	/*	(double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG ) */
	{ "push\teventid", 0, 0 },				/* 48 */
	{ NULL, 0, 0 },						/* 49 */
	/* Pops the value from the stack and adds it to array on the top of stack */
	/*	or pops both values from the stack, builds an array from them */
	/*	& pushes it on the stack if neither of them are arrays */
	{ "arra", 0, 0 },					/* 4a */
	/* Pops the top-of-stack value & sets the cause of usecode event handler to it
		(double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG )
		Used to call event handlers from the usecode
	*/
	{ "pop\teventid", 0, 0 },				/* 4b */
};

/* Embedded function table for Black Gate */
static const char* bg_func_table[] = 
{
	/* Random1(Hi) - Returns a random integer or from 1 to Hi inclusively
		Used to select a random party member for remarks
	*/
	"Random1",						/* 0 */
	/* ExecuteMacro(itemref, array).
		Executes a macro for this itemref.
		Macro is an array of opcodes (bytes) and operands (words)
		Macro opcodes:			
			either 0x35 or 0x45 - deletes an item (fire caused by the flaming oil)
			0x46 <integer> - sets item's frame number to integer (rotating crossbeam
						while opening the portcullis gate)
			0x52 <string> - the same as ItemSay() function
			0x55 <usecode function number> - call a usecode event for the item
			0x56 <integer> - Guardian's phrase with given number
		All spells use this function - if current weather is 3 - 606 is called
		instead of the second part of the spell engine (magic negation field at Ambrosia,
			Armageddon is blocked after shouting 1st mantra & before shouting second).
		Macro execution is asynchronous to the commands following ExecuteMacro -
			??? macro opcodes are executed one per redraw frame counted???
		Macro execution seems to be interruptable by opening inventory.
		The initial red moongate uses this to grow in height and then call another
		usecode function.
		This initial sequence is fired from Iolo's usecode hanlder.
	*/
	"ExecuteMacro",						/* 1 */
	/* ScheduleMacro(itemref, array, delay)
		The same as above - but schedules a macro for execution later
		Delay seems to be in frame counter units???
		(use a serpent venom on somebody - first raises stats, then - after some time
			- lower them)
		Can also shout exclamations - "Call it... Heads... Its tails..."
			- double-click on money.
	*/
	"ScheduleMacro",					/* 2 */
	/* SwitchTalkTo(itemref, face_frame) - switches conversation to other NPC,
		selecting one of possible face pictures
		Shows the face in the bottom of the screen if the face on top was not hidden yet
		Switching talk to -277 means - huge reg Guardian semi-transparent face
	*/
	"SwitchTalkTo",						/* 3 */
	/* HideNPC(itemref) - hides NPC face away from the conversation */
	"HideNPC",						/* 4 */
	/* AddAnswer(str_or_array) - adds a answer or set of answers to */
	/*	the set of talk threads suggested */
	"AddAnswer",						/* 5 */
	/* RemoveAnswer(str) - removes a given string from set of talk threads suggested */
	"RemoveAnswer",						/* 6 */
	/* SaveTalkStartNew() - saves the conversation state & starts */
	/*	new blank set of talk answers suggested */
	"SaveTalkStartNew",					/* 7 */
	/* RestoreTalk() - restores set of answers suggested */
	/* 	as was saved in SaveTalkStartNew() */
	"RestoreTalk",						/* 8 */
	/* EmptyTalk() - removes all talk answers
		Looks like the same as "cla" opcode
	*/
	"EmptyTalk",						/* 9 */
	/* GetAnswer() - asks user to select one of the talk threads & returns it
		Practically the same functionality as "ask" opcode
	*/
	"GetAnswer",						/* a */
	/* GetAnswerIndex() - asks user to select one of the talk threads
		& returns the 1-based index of it instead the text itself
	*/
	"GetAnswerIndex",					/* b */
	/* AskNumber(min, max, step, default) - asks user a number by showing a slider */
	/*	with given parameters */
	"AskNumber",						/* c */
	/* SetItemType(itemref, type) - changes a type of given item to the new value */
	"SetItemType",						/* d */
	/* AreItemsNearNPC(NPCID, type, distance) - returns boolean whether there are 
		items of such Type at the given distance from the given NPC.
		Distance -1 means - screen borders
	*/
	"AreItemsNearNPC",					/* e */
	/* PlaySoundEffect(sound_num) - plays a given effect soundtrack
	*/
	"PlaySoundEffect",					/* f */
	/* Random2(lo, hi) - returns a random integer from lo to hi inclusively */
	"Random2",						/* 10 */
	/* GetItemType(itemref) - returns a "type" value of the item */
	"GetItemType",						/* 11 */
	/* GetItemFrame(itemref) - returns a "frame" value of the item */
	"GetItemFrame",						/* 12 */
	/* SetItemFrame(itemref, frame) - sets a "frame" value for the item */
	"SetItemFrame",						/* 13 */
	/* GetItemQuality(itemref) - returns a "quality" value of the item */
	"GetItemQuality",					/* 14 */
	/* SetItemQuality(itemref, quality) - sets a "quality" value for the item
		(Penumbra's plaque)
		Returns old quality???
	*/
	"SetItemQuality",					/* 15 */
	NULL,							/* 16 */
	NULL,							/* 17 */
	/* GetItemCoords(itemref) - parameter is itemref as returned by
			ItemSelectModal(), returns array
		The array is 2 item's coordinates - array[1], array[2]
	*/
	"GetItemCoords",					/* 18 */
	NULL,							/* 19 */
	NULL,							/* 1a */
	/* GetNPCID(itemref) - returns NPCID from Itemref - possibly does-nothing function???
		Accepts values like -2 for Spark or -356 for Avatar on input
	*/
	"GetNPCID",						/* 1b */
	/* GetNPCClass(NPCID) - returns a number specifying the NPC class -
		fighter/sage/shopkeeper etc. Used in exclamations like "Oh my aching back!"
		10 seems to be Noble (Finnegan & his aching back :-) )
		Or maybe I'm wrong and this is GetNPCActivity???
	*/
	"GetNPCClass",						/* 1c */
	/* SetNPCActivity(NPCID, activity) - sets an NPC activity
		11 - Loiter (leave with "go home")
		15 - Wait (leave with "wait here")
		??? in cheat menu order?
	*/
	"SetNPCActivity",					/* 1d */
	/* JoinNPC(itemref) - joins NPC to the party
		This seems to only set Party flag on - the same effect as doing this in cheat mode
		???in BG, unusual companions (joined by cheat) will not be shown in Inventory/Ztats
			gumps. Where this information (whether the NPC is a usual companion) is kept?
		In SI, they will (I saw Rotoluncia's Ztats - though no 2 keys + a wand usually taken
			from her when dead in inventory. Trying to display Vasculio's Ztats hung
			the game) - but headless :-)
		Once more - it is kept somewhere...
	*/
	"JoinNPC",						/* 1e */
	/* DismissNPC(itemref) - dismisses NPC from the party */
	"DismissNPC",						/* 1f */
	/* GetNPCStat(NPCID, property_id) -
	 	returns NPC's statistic value (9 is food level, 0 is Strength, 3 is Hits, )
		(1 & 4 seems to be Training & Combat???)
		Vas Mani (Restoration) spell is a good example
	*/
	"GetNPCStat",						/* 20 */
	/* ChangeNPCStat(itemref, property_id, delta) - 
	 	increments (or decrements for negative delta) NPC's statistic value.
	*/
	"ChangeNPCStat",					/* 21 */
	/* AvatarNPCID() - returns Avatar's NPC ID */
	"AvatarNPCID",						/* 22 */
	/* GetPartyMembers() - returns the array of NPCs which are party members currently */
	/* 	??? NPC IDs returned??? */
	"GetPartyMembers",					/* 23 */
	/* CreateItem(Type) - creates an item of given Type, returns itemref
		Must be followed by SetItemFrame()???
	*/
	"CreateItem",						/* 24 */
	NULL,							/* 25 */
	/* InsertNewItem(Coords) Parameter is an array of c
	*/
	"InsertNewItem",					/* 26 */
	/* GetNPCName(npc_id_or_array) - returns an NPC name by NPC ID
		Returns an array of names if array was specified
	*/
	"GetNPCName",						/* 27 */
	/* Counts the amount of party gold if called as Func28(-357, 644, -359, -359)
		644 is for sure the Type of gold heap
		649 is for sure the Type of serpent venom
		839 for hourglass
		Last parameter seems to be the frame number of the items - -359 means "any"
	*/
	NULL,							/* 28 */
	/* Returns itemref of the given item in Avatar's (or party's??? - parameter 4???)
			posessions
		0 if no such item
		Func28(-357, 761, -359, -359) - spellbook.
		Func28(-357, 839, -359, 0) - Nicodemus's hourglass
		(known as Hourglass Of Fate in SI).
		Third parameter is quality
	*/
	NULL,							/* 29 */
	/* GetContainerItems(container_itemref, type, quality, ???) - returns an array */
	/* 	??? of itemrefs */
	"GetContainerItems",					/* 2a */
	/* Reduces the amount of party gold if called Func2b(delta, 644, -359, -359, true) 
		Reduces the amount of party eggs if called Func2b(delta, 377, -359, 24, true) 
	*/
	NULL,							/* 2b */
	/* Increases the amount of party gold if called Func2c(delta, 644, -359, -359, true) 
		Returns false if party is overloaded & cannot accept this
	*/
	NULL,							/* 2c */
	/* Not used at all */
	NULL,							/* 2d */
	/* PlayMusic(song#, itemref) - plays a MIDI music track. */
	/* 	Song# = 255 means - mute the MIDI channel. */
	"PlayMusic",						/* 2e */
	/* IsNPCInParty(npc_id) - returns true if given NPC is in party, */
	/*	otherwise returns false */
	"IsNPCInParty",						/* 2f */
	/* Takes a Type(???), returns an array of all visible items/NPCs of this type */
	NULL,							/* 30 */
	/* IsNPCxxx(itemref) - returns true if given NPC is ????? */
	NULL,							/* 31 */
	/* DisplaySign(gump#, array_of_strings) - displays a sign/plaque */
	/* 	of given gump number & given text (array of strings or single string). */
	"DisplaySign",					/* 32 */
	/* ItemSelectModal() - switches the engine to cross-cursor "use" mode.
	 	Returns an itemref of the item selected by user by single-click. 
	 	Does not return till user will single-click on something.
		The entity returned can be treated as array with coords at array[2], array[3]
	*/
	"ItemSelectModal",					/* 33 */
	/* Not used at all */
	NULL,							/* 34 */
	/* ItemsNearItem(itemref, type, distance, ???) - returns an array of items
			of given Type which are closer then Distance to the item specified by Itemref
			Parameter 4 is possibly frame number to compare???
			Type of -1 means - all NPC??? (Fear spell)
		(Powder keg & cannon ball near the cannon)
	*/
	"ItemsNearItem",					/* 35 */
	/* Takes an NPCID as a single parameter, seems to return the amount of free
		space in its inventory
	*/
	NULL,							/* 36 */
	NULL,							/* 37 */
	/* GetTimeHour() - returns hour part of current game time - 0-23 */
	"GetTimeHour",						/* 38 */
	/* GetTimeMinute() - returns minute part of current game time - 0-59 */
	"GetTimeMinute",					/* 39 */
	/* GetItemRef(NPCID) - returns Itemref from NPCID - possibly does-nothing function???
	*/
	"GetItemRef",						/* 3a */
	NULL,							/* 3b */
	/* GetNPCxxx(NPCID) - returns the same??? NPC mode as set in function 3d */
	NULL,							/* 3c */
	/* SetNPCxxx(NPCID, mode) - ???sets some NPC mode
		often called with mode=2 before entering combat
		???alignment
	*/
	NULL,							/* 3d */
	/* TeleportItem(itemref, array) - teleports an NPC
		array[1] - X coord
		array[2] - Y coord
		array[3] - height
		Help spell calls this with (3a8, 47a, 0) array - LB throne room
		Recall spell calls this with (5aa, 500, 0) array (House of the Dead) - in some cases.
		Also interesting location - (217, 489, 0) - back from the Sphere.
		Function 6cf teleports there is Sphere is not destroyed & party does not
			posesses item 0x347
			(the hourglass, maybe parameter 4 in 0x28 checks the Frame number to
				check whether the hourglass is enchanted???
			BTW - there is a known bug in BG that sometimes you can enter the Sphere
				with non-enchanted hourglass - cause???)
		What are these locations?
	*/
	"TeleportItem",						/* 3e */
	/* VanishNPC(NPCID) - only parameter is NPCID, no return value
		Called in eventid 2 for the plaque - never mind what - and NPC -23 (LB)
			- plaque falling on LB's head
		Called on Weston (-69) when LB frees him
		Called on Batlin (GetNPCID(-26)) when he vanishes, feeling the Cube at Avatar's
			posession
		Called on -214 in Vesper - when Yvella want to tell Catherine's father & Mara
			something....
		Called on Kreg/Kraig(-245) after he laughs when invis. potion is given to him
		Called on Addom (-164) after Brion attaches to crystal & completes the orrery
			viewer
		Called on -156 (Balayna???) when Rankin says he did not see Balayna for some time
		Called on all Scara Brae ghosts in For Each loop when Forsythe jumps into the Well
		The exact logic is not clear - what appears with the vanished NPC?
		Where it is teleported???
	*/
	"VanishNPC",							/* 3f */
	/* ItemSay(itemref, str) - displays a string on the screen near the specified item */
	"ItemSay",						/* 40 */
	NULL,							/* 41 */
	/* GetItemZCoord(itemref) - returns Z coordinate of the item */
	"GetItemZCoord",					/* 42 */
	/* SetItemZCoord(itemref, coord) - sets a Z coordinate for the item */
	"SetItemZCoord",					/* 43 */
	/* GetWeather() - returns current weather - 0-3
		3 seems to negate magic
	*/
	"GetWeather",						/* 44 */
	/* SetWeather(weather) - sets new current weather - 0-3 */
	"SetWeather",						/* 45 */
	/* Sits down NPC??? */
	NULL,							/* 46 */
	/* SummonCreature(Type, boolean) - engine under Kal Bet Xen (Swarm), Kal Xen (???)
		an Kal Vas Xen (Summon).
		Summons a creature of given Type. Second parameter is true only for Kal Vas Xen
		Return value???
	*/
	"SummonCreature",					/* 47 */
	/* Shows a map of Britannia. Double-click on the map, Vas Wis (Peer) spell or reading
		the Brommer's Britannia book
	*/
	"ShowMap",						/* 48 */
	/* KillNPC(itemref) - kills the given NPC. Owen's suicide is this function alone.
		Plaque falling on LB's head - KillNPC, then VanishNPC.
		Balayna takes the vial from you & dies - this function alone
		Death bolt spell uses it too.
		These are all occurences.
	*/
	"KillNPC",						/* 49 */
	/* Some kind of comparing 2 numbers (2 params) - returns boolean
		Used to compare somebody' Strength to item's Quality
	*/
	"???",							/* 4a */
	/* SetNPCAttackMode(NPCID, mode) - sets a combat mode for the NPC
		Mode 7 is Fleeing - others are in men
		Fear spell, using dirty diapers on people
			(it is the same as Fear spell) - mode 7
		LB attacking in an LB cheat room - mode 0
	*/
	"SetNPCAttackMode",					/* 4b */
	/* SetTargetNPCToAttack(attacker_NPCID, target_NPCID) - sets target NPC to attack
		??? how it is called in a cheat menu?
		Usually called with Avatar as target NPC
	*/
	"SetTargetNPCToAttack",			/* 4c */
	/* CloneNPC(NPCID) - clones an NPC - Clone spell
		Returns something - return value never used.
	*/
	"CloneNPC",						/* 4d */
	/* Not used at all */
	NULL,							/* 4e */
	/* ShowCrystalBall(coords_array) - shows a crystal ball view of a given world point
	*/
	"ShowCrystalBall",					/* 4f */
	/* ShowWizardEye(parm1, parm2) - shows a view for telescope or Wizard Eye spell
		For telescope - ShowWizardEye(10000, 1000)
		For Wizard Eye spell - ShowWizardEye(45, 200)
	*/
	"ShowWizardEye",					/* 50 */
	/* ResurrectNPC(itemref) - resurrects an NPC. Itemref is a dead body's itemref
		Returns false is cannot resurrect - LB in this case says "Alas..." and then
			about burial.
		???what is the cause of such LB's behaviour? When ResurrectNPC returns false?
	*/
	"ResurrectNPC",						/* 51 */
	/* AddSpellToBook(spellnum, 0, spellbook_itemref) - Adds a new spell to the spellbook
		Returns false if there was already such spell in the book
		Maybe calling with 1 (never called such in BG) will remove the spell from the book?
	*/
	"AddSpellToBook",					/* 52 */
	/* ExecuteSprite(sprite, coordx, coordy, speedx, speedy, ???, ???)
		- executes an explosion-like sprite
		First parameter is sprite number in SPRITES.VGA
		speedx & speedy are non zero is the sprite floats away (smoke from smokebomb)
		Last parameters is usually -1 (not so for smokebomb)
	*/
	"ExecuteSprite",					/* 53 */
	/* Powder keg burst??? */
	NULL,							/* 54 */
	/* DisplayBook(itemref) - Displays book or scroll
		Text will be displayed further by "say" opcode
	*/
	"DisplayBook",						/* 55 */
	/* StopTime(???) - freezes all NPCs for some time. Parameter is duration???? 
		Type 6be object (Egg???) calls this with its Quality as a parameter on eventid 3
		Also called in Stop Time spell with the parameter 100
	*/
	"StopTime",						/* 56 */
	/* CauseLight(duration) - Glimmer/Light/Great Light spell logic
		duration is 110 for Glimmer, 500 for Light, 5000 for Great Light
	*/
	"CauseLight",						/* 57 */
	/* itemref as a parameter, returns some boolean... 
		???NPC is on barge
		At least true return from this blocks Mark spell - even mantra is not shouted
	*/
	"???",							/* 58 */
	/* CauseEarthquake(???) - causes an earthquake.
		Parameter is 40 for Armageddon spell & variable for Tremor spell
		Also called from func 85e (Forsythe jumping in the Well of Souls) - with parameter 15
	*/
	"CauseEarthquake",					/* 59 */
	/* IsPlayerFemale() - returns 0 if male, 1 if female */
	"IsPlayerFemale",					/* 5a */
	/* CauseArmageddon() - all logic of Armageddon spell except
		shouting mantras, weather change, earthquake, & setting the game flag which
		affects Batlin's & LB's behaviour.
		Walks through the all NPC list & makes all of the Dead with Hits < 0
		Called also in LB's cheat room - "Busted, you thieving scoundrel bastard!"
	*/
	"CauseArmageddon",					/* 5b */
	/* Sets NPC to some state - like VanishNPC() or KillNPC() */
	"???",							/* 5c */
	/* CauseBlackout() - darken the whole screen for some time */
	"CauseBlackout",					/* 5d */
	/* ArraySize(a) - returns number of elements in the array */
	"ArraySize",						/* 5e */
	/* Something for Mark spell??? */
	"???",							/* 5f */
	/* Called only once, something in a Recall spell
		Called after setting activity to 31 (Follow Avatar) to all party members
		and resetting flag 0x39 (what is it???)
		No return value, single parameter which is itemref
	*/
	NULL,							/* 60 */
	/* Called only once, parameters are NPC strength, 0, 12, 3
		Return value exists but not used
	*/
	NULL,							/* 61 */
	/* IsUnderTheRoof() - returns true if the party is under the roof.
		Sextant will not work under the roof.
		Also used in some place (???) where it causes a shout "Try it outside!"
	*/
	"IsUnderTheRoof",					/* 62 */
	/* SetOrreryState(coords_array, status) - sets the state of the planets in Brion's orrery.
		Coords are usually ordinary orrery coords - array of 2 integers. (59c, b4c)
		status is a small integer which influences the state of the planets.
	*/
	"SetOrreryState",					/* 63 */
	/* Not used at all */
	NULL,							/* 64 */
	/* GetTimerElapsedHours(timer_number) - returns number of hours of game
		time elapsed since the time set in the timer.
		Timer valus are persistent beyound doubt - maybe GAMETIM(A) files?
		Timer 0x0 - Kliftin at Jhelom making the flag of Honor
		Timers 0x1 - Bennie in LB's house giving free meal
		Timers 0x2-0x4 - Martina, Wench and Roberto at Buc. Den
		Timer 0x5 - called from 0x6c3 code deletes all light sources/candles,
			serpentine daggers, buckets, victims & bloods - Minoc murder scene.
			A well-known game bug! Proximity usecode Egg 0x6c3 is just at
			the center of the murder scene with firing distance of 16
			(which is exactly the sawmill entrance)
			The logic is: if flag 0x122 is not set, set it, and then set the timer
			0x5.
			Else - if the flag is already set and if 24 game hours elapsed
			from timer 0x5 - delete all murder scene objects in the distance
			15 aroung the Egg and the Egg iself.
			So - entering the sawmill second time after more a day from the first
			time entering it will delete the murder scene.
			The bug is that sometimes it deletes just at the first time and
			the player never sees it and cannot pick a dagger.
		Timer 0x6 - the same with Alagner's body (victim).
		Timer 0x8 - poisoning Balayna by Rankin.
		Timer 0xa - Jaana healing.
			Another well-known game bug! 
			If you ask Jaana to heal when she is in the party, and if flag 0x29
			is false, than the "period" is set to 5. Otherwise, the "period" is
			set to the time elapsed since timer 0xa.
			Then, if the period < 4, say "I am sorry..."
			Otherwise, Jaana heals - this sets flag 0x29 and timer 0xa.
			The bug is that Jaana heals only once per game - and than always
				says "I am sorry..."
		Cause of both bugs: game timers are broken beyound doubt. Seriously.
			Something like GetTimerElapsedHours for 0xa always
			returns 0 or at least value < 4 and GetTimerElapsedHours for 0x5
			always returns >= 24
			Too sad. No chances of fixing it using usecode patches.
		Timer 0xb - the last thievery act by the Avatar. (where is it set???)
		These are all used timers.
	*/
	"GetTimerElapsedHours",					/* 65 */
	/* SetTimer(timer_number) - sets the timer value to the current game time
	*/
	"SetTimer",						/* 66 */
	/* Is Avatar wearing a fellowship medallion???? */
	NULL,							/* 67 */
	/* IsMousePresent() - returns true if mouse is present, false otherwise
	*/
	"IsMousePresent",					/* 68 */
	/* GetSpeechTrack() - returns a speech track number previously set by SetSpeech()
	*/
	"GetSpeechTrack",					/* 69 */
	NULL,							/* 6a */
	NULL,							/* 6b */
	NULL,							/* 6c */
	NULL,							/* 6d */
	NULL,							/* 6e */
	/* DeleteItem(itemref) - deletes an item
	*/
	"DeleteItem",						/* 6f */
	/* Called only once after initial conversation of just-arrived Avatar
		with Iolo
	*/
	NULL,							/* 70 */
	NULL,							/* 71 */
	NULL,							/* 72 */
	NULL,							/* 73 */
	NULL,							/* 74 */
	/* StartEndGame(boolean) - TRUE is successful endgame (wand against the gate),
		FALSE is unsuccessful (passing through the gate)
	*/
	"StartEndGame",						/* 75 */
	/* FireCannon(cannon_itemref, fire_direction, ball_type, ???,
						cannon_type, cannon_type)
		- fires a cannon in specified direction
	*/
	"FireCannon",						/* 76 */
	NULL,							/* 77 */
	NULL,							/* 78 */
	NULL,							/* 79 */
	NULL,							/* 7a */
	NULL,							/* 7b */
	NULL,							/* 7c */
	NULL,							/* 7d */
	/* PlaySpeech() - plays a speech track set by SetSpeech()
	*/
	"PlaySpeech",						/* 7e */
	NULL,							/* 7f */
	NULL,							/* 80 */
	NULL,							/* 81 */
	NULL,							/* 82 */
	NULL,							/* 83 */
	NULL,							/* 84 */
	NULL,							/* 85 */
	NULL,							/* 86 */
	NULL,							/* 87 */
	/* Function 0x88 - GetNPCFlag(Itemref, flagno).
		Flagno 1 is Slept
		Flagno 8 is Poisoned 
		Flag 25 causes people to say "Oink!"
		Returns boolean
		Or maybe not only NPC flag? - see the very first function on sails & gangplanks...
		??? Suggestion. Flagno 10 is "on barge" flag which is set when the NPC is unmovable
		and mouse controls barge itself (sitting on ship/carpet/cart). A reasonable one.
		Needs checking.
		*/
	"GetNPCFlag",						/* 88 */
	/* Function 0x89 - SetNPCFlag(itemref, flagno).
		Flagno 1 is Slept
		Flagno 8 is Poisoned
		Sets flag to true
		*/
	"SetNPCFlag",						/* 89 */
	/* Function 0x8a - ResetNPCFlag(itemref, flagno).
		Flagno 1 is Slept
		Flagno 8 is Poisoned
		Sets flag to false
		*/
	"ResetNPCFlag",						/* 8a */
	NULL,							/* 8b */
	NULL,							/* 8c */
	NULL,							/* 8d */
	NULL,							/* 8e */
	NULL,							/* 8f */
	NULL,							/* 90 */
	NULL,							/* 91 */
	NULL,							/* 92 */
	/* Itemref as parameter, returns an array of NPCs which are then passed to
		ResurrectNPC()
		called only once */
	NULL,							/* 93 */
	/* Called only for orrery viewer
		Large orrery coords as parameters,
		called only once and followed by ShowCrystalBall() at the orrery location
	*/
	"SetupOrrery",						/* 94 */
	/* Some spells like An Flam */
	NULL,							/* 95 */
	/* Appears only in crazy talk after answering copy-protection incorrectly */
	NULL							/* 96 */
};

/*
 Embedded function table pointer
 TODO: set to bg_func_table or si_func_table depending on the command line
*/
const char** func_table = bg_func_table;
int func_table_size = sizeof(bg_func_table);

/* Functions */

/* Prints module's data segment */
void process_data_seg(FILE* f, unsigned short ds)
{
	long pos;
	unsigned short off = 0;
	unsigned char* p;
	unsigned char* pp;
	unsigned char* tempstr;
	/* Allocate a temporary buffer */
	tempstr = malloc(70 + 1);
	pos = ftell(f);
	pp = p = malloc(ds);
	fread(p, 1, ds, f);
	fseek(f, pos, SEEK_SET);
	/* Print all strings & their offsets */
	while( off < ds )
	{
		int len;
		unsigned short localoff = 0;
		/* Print all parts of the string - wrapping them around */
		while( (len = ( strlen(pp) > 70 )) ? 70 : strlen(pp) )
		{
			/* TODO! Escape characters if ' is part of the string */
			memcpy(tempstr, pp, len);
			tempstr[len] = '\0';
			if( localoff )
				printf("\tdb\t\'%s\'\n", tempstr);
			else
				printf("%04X\tdb\t\'%s\'\n", off, tempstr);
			localoff += len; 
			pp += len;
		}
		pp++;
		off += localoff + 1;
		printf("\tdb\t00\n");
	}
	free(p);
	free(tempstr);
}

/*
 Prints single opcode
 Return number of bytes to advance the code pointer
 Prints first characters of strings referenced
*/
unsigned short print_opcode(unsigned char* ptrc, unsigned short coffset,
							unsigned char* pdataseg,
							unsigned short* pextern,
							unsigned short externsize,
							unsigned char* opcode_buf,
							unsigned char* intrinsic_buf,
							int mute, int count_all_opcodes,
							int count_all_intrinsic)
{
	unsigned short nbytes;
	unsigned short i;
	opcode_desc* pdesc;
	if( count_all_opcodes )
		opcode_buf[*ptrc]++;
	/* Find the description */
	pdesc = ( *ptrc >= ( sizeof(opcode_table) / sizeof( opcode_desc ) ) ) ?
												NULL : opcode_table + ( *ptrc );
	if( pdesc && ( pdesc->mnemonic == NULL ) )
		/* Unknown opcode */
		pdesc = NULL;
	if( ( pdesc == NULL ) && !count_all_opcodes )
		/* Unknown opcode */
		opcode_buf[*ptrc]++;
	/* Number of bytes to print */
	nbytes = pdesc ? ( pdesc->nbytes + 1 ) : 1;
	/* Print label */
	if( !mute )
		printf("%04X: ", coffset);
	/* Print bytes */
	for( i = 0; i < nbytes; i++ )
		if( !mute )		
			printf("%02X ", ptrc[i]);
	if( !mute )
	{
		/* Print mnemonic */
		if( nbytes < 4 )
			printf("\t");
		if( nbytes > 6 )
			printf("\n\t\t");
		printf("\t%s", pdesc ? pdesc->mnemonic : "???");
	}
	/* Print operands if any */
	if( ( nbytes == 1 ) || ( pdesc == NULL ) )
	{
		if( !mute )
			printf("\n");
		return nbytes;
	}
	switch( pdesc->type )
	{
	case IMMED:
		/* Print immediate operand */
		if( !mute )
			printf("\t%04XH\t\t\t; %d\n", *(unsigned short*)( ptrc + 1 ),
								*(short*)( ptrc + 1 ));
		break;
	case IMMED_BYTE:
		/* Print immediate operand */
		if( !mute )
			printf("\t%02XH\t\t\t; %d\n", (unsigned short)(ptrc[1]),
								(unsigned short)(ptrc[1]));
		break;
	case ( VARREF | RELATIVE_JUMP ):
		/* NEXT command */
		if( !mute )
		{
			/* Print variable reference */
			printf("\t[%04X], ", *(unsigned short*)( ptrc + 1 ));
			/* Print variable reference */
			printf("[%04X], ", *(unsigned short*)( ptrc + 3 ));
			/* Print variable reference */
			printf("[%04X], ", *(unsigned short*)( ptrc + 5 ));
			/* Print variable reference */
			printf("[%04X], ", *(unsigned short*)( ptrc + 7 ));
			/* Print jump desination */
			printf("%04X\n", *(short*)( ptrc + 9 ) + (short)coffset + nbytes);
		}
		break;
	case ( IMMED | RELATIVE_JUMP ):
		/* JMPA command */
		if( !mute )
		{
			/* Print immediate operand */
			printf("\t%04XH,\t", *(unsigned short*)( ptrc + 1 ));
			/* Print jump desination */
			printf("%04X\t", *(short*)( ptrc + 3 ) + (short)coffset + nbytes);
			/* Print immediate operand */
			printf("; %d\n", *(short*)( ptrc + 1 ));
		}
		break;
	case DATA_STRING:
		if( !mute )
		{
			unsigned char* pstr;
			int len;
			/* Print data string operand - first characters only */
			pstr = pdataseg + *(unsigned short*)( ptrc + 1 );
			len = strlen(pstr);
			if( len > 20 )
				len = 20 - 3;
			printf("\t%04XH\t\t\t; ", *(unsigned short*)( ptrc + 1 ));
			for( i = 0; i < len; i++ )
				printf("%c", pstr[i]);
			if( len < strlen(pstr) )
				/* String truncated */
				printf("...");
			printf("\n");
		}
		break;
	case RELATIVE_JUMP:
		/* Print jump desination */
		if( !mute )
			printf("\t%04X\n", *(short*)( ptrc + 1 ) + (short)coffset + 3);
		break;
	case CALL:
		{
			/* Print call operand */
			unsigned short func = *(unsigned short*)( ptrc + 1 );
			if( ( func < ( func_table_size / sizeof(const char *) ) ) &&
				 func_table[func] )
			{
				/* Known function */
				if( !mute )
					printf("\t_%s@%d\t%s; %04X\n", func_table[func], ptrc[3],
						( strlen(func_table[func]) > 12 ) ? "" : "\t",
						func);
				if( count_all_intrinsic )
					intrinsic_buf[func]++;
			}
			else
			{
				/* Unknown function */
				if( func >= 256 )
					/* TODO: error handling here */
					printf("Unknown intrisinc function greater then 256!\n");
				intrinsic_buf[func]++;
				if( !mute )
					printf("\t%04X, %d\n", func, ptrc[3]);
			}
		}
		break;
	case EXTCALL:
	case ( VARREF | EXTCALL ):
		{
			unsigned short externpos = *(unsigned short*)( ptrc + 1 );
			/* Print extern call */
			if( pdesc->type & VARREF )
			{
				unsigned short externpos = *(unsigned short*)( ptrc + 1 );
				if( externpos < externsize )
				{
					if( !mute )
						printf("\textern:[%04X]\t\t; %04XH\n",
							externpos, pextern[externpos]);
				}
				else
					printf("\tBad extern table!\n");
			}
			else if( !mute )
				printf("\textern:%04X\t\t\n", externpos);
		}
		break;
	case VARREF:
		/* Print variable reference */
		if( !mute )
			printf("\t[%04X]\n", *(unsigned short*)( ptrc + 1 ));
		break;
	case FLGREF:
		/* Print game flag reference */
		if( !mute )
			printf("\tflag:[%04X]\n", *(unsigned short*)( ptrc + 1 ));
		break;
	default:
		/* Unknown type */
		if( !mute )
			printf("\n");
		break;
	}
	return nbytes;
}

void process_code_seg(FILE* f, unsigned short ds, unsigned short s, unsigned char* opcode_buf,
							unsigned char* intrinsic_buf,
							int mute, int count_all_opcodes,
								int count_all_intrinsic)
{
	long pos;
	unsigned short size;
	unsigned short externsize;
	unsigned short i;
	unsigned short offset;
	unsigned short nbytes;
	unsigned char* p;
	unsigned char* pp;
	unsigned char* pdata;
	unsigned short* pextern;
	pos = ftell(f);
	size = s - ds - sizeof(unsigned short);
	pp = p = malloc(size);
	pdata = malloc(ds);
	fread(pdata, 1, ds, f);
	if( !mute )
		printf("Code segment at file offset %08lXH\n", ftell(f));
	fread(p, 1, size, f);
	fseek(f, pos, SEEK_SET);
	/* Print code segment header */
	if( size < 3 * sizeof(unsigned short) )
	{
		printf("Code segment bad!\n");
		free(p);
		free(pdata);
		return;
	}
	/* Print argument counter */
	if( !mute )
		printf("\t\t.argc %04XH\n", *(unsigned short*)pp);
	pp += sizeof(unsigned short);
	/* Print locals counter */ 
	if( !mute )
		printf("\t\t.localc %04XH\n", *(unsigned short*)pp);
	pp += sizeof(unsigned short);
	/* Print externs section */
	externsize = *(unsigned short*)pp;
	if( !mute )
		printf("\t\t.externsize %04XH\n", externsize);
	pp += sizeof(unsigned short);
	if( size < ( ( 3 + externsize ) * sizeof(unsigned short) ) )
	{
		printf("Code segment bad!\n");
		free(p);
		free(pdata);
		return;
	}
	size -= ( ( 3 + externsize ) * sizeof(unsigned short) );
	pextern = (unsigned short*)pp;
	for( i = 0; i < externsize; i++ )
	{
		if( !mute )
			printf("\t\t.extern %04XH\n", *(unsigned short*)pp);
		pp += sizeof(unsigned short);
	}
	offset = 0;
	/* Print opcodes */
	while( offset < size )
	{
		nbytes = print_opcode(pp, offset, pdata, pextern, externsize,
							opcode_buf, intrinsic_buf, mute,
							count_all_opcodes,
							count_all_intrinsic);
		pp += nbytes;
		offset += nbytes;
	}
	free(p);
	free(pdata);
}

void process_func(FILE* f, long func, int i, int* found, unsigned char* opcode_buf,
							unsigned char* intrinsic_buf,
							int scan_mode,
							unsigned long opcode,
							unsigned long intrinsic)
{
	unsigned short s, ds, funcnum;	
	long off, bodyoff;
	/* Save start offset */
	off = ftell(f);
	/* Read function header */
	fread(&funcnum, sizeof(unsigned short), 1, f);
	fread(&s, sizeof(unsigned short), 1, f);
	/* Save body offset */
	bodyoff = ftell(f);
	fread(&ds, sizeof(unsigned short), 1, f);
	if( ( ( func == -1 ) || scan_mode ) && ( opcode == -1 ) && ( intrinsic == -1 ) )
		/* Only for general list & scan mode */
		printf("\tFunction #%d (%04XH), offset = %08lx, size = %04x, data = %04x\n", i,
								funcnum, off, s, ds);
	if( ( funcnum == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
	{
		/* Only for matching function or in one of the scan modes */
		if( funcnum == func )
		{
			*found = 1;
			printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
				"\t.msize\t%04XH\n\t.dsize\t%04XH\n",
				off, funcnum, s, ds);
		}
		/* Dump function contents */
		if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
			process_data_seg(f, ds);
		if( opcode != -1 ) 
			memset(opcode_buf, 0, 256);
		if( intrinsic != -1 ) 
			memset(intrinsic_buf, 0, 256);
		process_code_seg(f, ds, s, opcode_buf, intrinsic_buf,
				scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
				( opcode != -1 ), ( intrinsic != -1 ));
		if( ( ( opcode != -1 ) && opcode_buf[opcode] > 0 ) ||
			( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
		{
			/* Found */
			*found = 1;
			if( intrinsic != -1 )
				printf("\tFound function (%04XH) - %d times\n", funcnum,
								intrinsic_buf[intrinsic]);
			else
				printf("\tFound function (%04XH) - %d times\n", funcnum,
									opcode_buf[opcode]);
		}
	}
	/* Seek back, then to next function */
	fseek(f, bodyoff, SEEK_SET);
	fseek(f, s, SEEK_CUR);
}

int main(int ac, char** av)
{
	/* Preset to no match */
	unsigned long func = -1;
	const char* funcstr = NULL;
	unsigned char* opcode_buf = NULL;
	unsigned char* intrinsic_buf = NULL;
	long sz;
	int i = 0;
	int found = 0;
	int mode = 0;
	unsigned long opcode = -1;
	unsigned long intrinsic = -1;
	FILE* f;
	printf("Ultima 7 usecode disassembler v0.7\n\n");
	/* Parse command line */
	if( ac == 3 )
	{
		if( !strcmp(av[1], "-o") )
		{
			char* stopstr;
			/* Opcode search */
			opcode = strtoul(av[2], &stopstr, 16);
			if( stopstr - av[2] < strlen(av[2]) )
				opcode = -1;
			else
				/* Hex opcode OK */
				mode = 4;
		}
		else if( !strcmp(av[1], "-i") )
		{
			char* stopstr;
			/* Intrinsic function search */
			intrinsic = strtoul(av[2], &stopstr, 16);
			if( stopstr - av[2] < strlen(av[2]) )
				intrinsic = -1;
			else
				/* Hex opcode OK */
				mode = 5;
		}
	}
	else if( ac == 2 )
	{
		if( !strcmp(av[1], "-l") )
			/* List mode */
			mode = 2;
		else if( !strcmp(av[1], "-c") )
			/* Opcode scan mode */
			mode = 3;
		else
		{
			char* stopstr;
			/* Disassembly mode */
			funcstr = av[1];
			func = strtoul(funcstr, &stopstr, 16);
			if( stopstr - funcstr < strlen(funcstr) )
				/* Invalid number */
				func = -1;
			else
				mode = 1;
		}
	}
	if( mode == 0 )
	{
		printf("Usage:\n");
		printf("\tucdump -l - prints list of all present functions\n");
		printf("\tucdump -c - scans the whole usecode file for unknown opcodes\n");
		printf("\tucdump -o <hex number> - prints list of functions which use ");
		printf("the given opcode\n");
		printf("\tucdump -i <hex number> - prints list of functions which use ");
		printf("the given intrinsic function\n");
		printf("\tucdump <hex number> - disassembles single function to stdout\n");
		return -1;
	}
	/* Allocate opcode & intrinsic function buffers */
	if( mode != 2 )
	{
		opcode_buf = (unsigned char*)malloc(256);
		intrinsic_buf = (unsigned char*)malloc(256);
		if( ( opcode_buf == NULL ) || ( intrinsic_buf == NULL ) )
		{
			/* No memory */
			if( opcode_buf )
				free(opcode_buf);
			if( intrinsic_buf )
				free(intrinsic_buf);
			printf("Out of memory\n");
			return -2;
		}
		/* Zero them */
		memset(opcode_buf, 0, 256);
		memset(intrinsic_buf, 0, 256);
	}
	/* Open a usecode file */
#ifdef _WIN32
	/* Microsoftism */
	f = fopen("usecode", "rb");
#else
	f = fopen("usecode", "r");
#endif
	if( f == NULL )
	{
		/* Free the buffers */
		if( opcode_buf )
			free(opcode_buf);
		if( intrinsic_buf )
			free(intrinsic_buf);
		printf("Failed to open usecode file\n\n");
		return -2;
	}
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if( mode == 1 )
		printf("Looking for function number %08lx\n\n", func);
	while( ftell(f) < sz )
	{		
		process_func(f, func, i, &found, opcode_buf, intrinsic_buf, ( mode == 3 ),
									opcode, intrinsic);
		if( ( ( mode != 4 ) && ( mode !=5 ) ) || found )
			i++;
		if( ( mode == 4 ) || ( mode == 5 ) )
			found = 0;
	}
	if( func == -1 )
	{
		if( ftell(f) != sz )
			printf("Problem, tell = %ld!\n", ftell(f));
		printf("Functions: %d\n", i);
	}
	if( ( ( mode == 1 ) || ( mode == 4 ) ) && !found )
		printf("Function not found.\n");
	fclose(f);
	/* Dump unknowns */
	if( ( mode == 1 ) || ( mode == 3 ) )
	{
		if( opcode_buf )
		{
			int found = 0;
			for( i = 0; i < 255; i++ )
				if( opcode_buf[i] )
				{
					if( !found )
					{
						printf("Undefined opcodes found\n");
						found = 1;
					}
					printf("0x%02lx (%d times)\n", i, opcode_buf[i]);
				}
		}
		if( intrinsic_buf )
		{
			int found = 0;
			for( i = 0; i < 255; i++ )
				if( intrinsic_buf[i] )
				{
					if( !found )
					{
						printf("Undefined intrinsic functions found\n");
						found = 1;
					}
					printf("0x%02lx (%d times)\n", i, intrinsic_buf[i]);
				}
		}
	}
	/* Free the buffers */
	if( opcode_buf )
		free(opcode_buf);
	if( intrinsic_buf )
		free(intrinsic_buf);
	return 0;
}
