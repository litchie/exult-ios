#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "opcodes.h"
#include <cstdlib>

/* Opcode table - common to BG & SI */
const opcode_desc opcode_table[] =
{
  { NULL, 0, 0 },           /* 00 */
  { NULL, 0, 0 },           /* 01 */
  /* Next iteration of For Each-style loop
    Operands are 1,2, <current>, <array variable>, <jump>
    <current> is set to next value from <array variable>
    Jumps to <jump> if array ended
    ??? what are 1,2? Are they used?
  */
  { "next", 10, EFF_RELATIVE_JUMP },         /* 02 */
  { NULL, 0, 0 },           /* 03 */
  /* Asks user to select one the talk answers
    Jumps where specified if no answer available
  */
  { "ask", 2, EFF_RELATIVE_JUMP },        /* 04 */
  /* Pops a value from the top of stack, jump if false, zero or empty array */
  { "jne", 2, EFF_RELATIVE_JUMP },        /* 05 */
  /* jump */
  { "jmp", 2, EFF_RELATIVE_JUMP },        /* 06 */
  /* Pops the top-of-stack string & jumps if the string is NOT the last talk answer 
    ??? first operand seems to be always 1
  */
  { "jmpa", 4, EFF_RELATIVE_JUMP },      /* 07 */
  { NULL, 0, 0 },           /* 08 */
  /* Adds two values on the stack, popping them & pushing result
    Can be used to add integer to string - in this case integer is converted to string 
    and 2 strings are concatenated */
  { "add", 0, EFF_BIMATH },          /* 09 */
  /* Decrements second value of the stack by first value, popping them & pushing result */
  { "sub", 0, EFF_BIMATH },          /* 0a */
  /* Divides second value of the stack by first value, popping them & pushing result */
  { "div", 0, EFF_BIMATH },          /* 0b */
  /* Multiplies two values on the stack, popping them & pushing result */
  { "mul", 0, EFF_BIMATH },          /* 0c */
  /* Divides second value of the stack by first value, popping them & pushing reminder */
  { "mod", 0, EFF_BIMATH },          /* 0d */
  /* Boolean AND on two values on stack, popping them & pushing result
    Top-of-stack variable is the left one in case of string addition */
  { "and", 0, EFF_BIMATH },          /* 0e */
  /* Boolean OR on two values on stack, popping them & pushing result */
  { "or", 0, EFF_BIMATH },           /* 0f */
  /* Inverts a boolean value on top of stack */
  { "not", 0, EFF_UNIMATH },          /* 10 */
  { NULL, 0, 0 },           /* 11 */
  /* Pops a stack value to given local variable */
  { "pop", 2, EFF_POP },         /* 12 */
  /* Pushes a TRUE boolean value on the stack */
  { "push\ttrue", 0, EFF_PUSH },         /* 13 */
  /* Pushes a FALSE boolean value on the stack */
  { "push\tfalse", 0, EFF_PUSH },        /* 14 */
  { NULL, 0, 0 },           /* 15 */
  /* Pops 2 values from the stack, pushes boolean value -
    TRUE if second value "?" then first 
    (where "?" can be greater, greater or equal etc...)
  */
  { "cmpgt", 0, EFF_CMP },          /* 16 */
  { "cmplt", 0, EFF_CMP },          /* 17 */
  { "cmpge", 0, EFF_CMP },          /* 18 */
  { "cmple", 0, EFF_CMP },          /* 19 */
  { "cmpne", 0, EFF_CMP },          /* 1a */
  { NULL, 0, 0 },         /* 1b */
  /* Adds a string from data segment to string register current contents */
  { "addsi", 2, EFF_STUPIDEFF },        /* 1c */
  /* Pushes a string value given by 16bit string offset in data segment to stack */
  { "pushs", 2, EFF_STUPIDEFF },        /* 1d */
  /* Pops specified number of values from stack, builds an array from them
    & pushes it on the stack 
    Pushes the empty array to top of the stack if operand is 0
  */
  { "arrc", 2, EFF_STUPIDEFF },         /* 1e */
  /* Pushes immediate 16bit integer to stack */
  { "pushi", 2, EFF_PUSH },          /* 1f */
  { NULL, 0, 0 },           /* 20 */
  /* Pushes a local variable on stack */
  { "push", 2, EFF_PUSH },          /* 21 */
  /* Compares 2 values on the stack, pops them & pushes TRUE if they are equal */
  { "cmpeq", 0, EFF_CMP },          /* 22 */
  { NULL, 0, 0 },           /* 23 */
  /* Calls a usecode function - function number is 0-based index to externs array */
  { "call", 2, EFF_SINGLELINE },          /* 24 */
  /* Return from function without result returned on the stack */
  { "ret", 0, EFF_SINGLELINE },          /* 25 */
  /* Uses the top-of-stack value to index (1-based) the array variable, 
    pops index & pushes result - the value from the array */
  { "aget", 2, EFF_SINGLELINE },          /* 26 */
  { NULL, 0, 0 },           /* 27 */
  { NULL, 0, 0 },           /* 28 */
  { NULL, 0, 0 },           /* 29 */
  { NULL, 0, 0 },           /* 2a */
  { NULL, 0, 0 },           /* 2b */
  /* ??? Looks like to be the same as "exit"
    ??? Suggestion: maybe "exit" is for functions, while "exit2" is for event handlers? */
  { "exit2", 0, EFF_EXIT | EFF_SINGLELINE },          /* 2c */
  /* Pop the top-of-stack value & sets it as a return value */
  { "popr", 0, EFF_STUPIDEFF },         /* 2d */
  /* Opens a new For Each-style enumeration loop.
    Always followed by "next" opcode??? */
  { "enum", 0, 0 },           /* 2e */
  /* Adds local variable's string value to string register current contents */
  { "addsv", 2, EFF_STUPIDEFF },         /* 2f */
  /* If (top-of-stack - 1) value is in top-of-stack array, pushes true, 
    otherwise pushes false - after popping array & value */
  { "in", 0, EFF_STUPIDEFF },           /* 30 */
  /* Something strange with 2 varrefs (???) as operands
    Appears in BG only - in talk to Raymundo in the Theatre - trying to sing a song
    I -am- the Avatar (function: 08D1H)
  */
  { "???", 4, 0 },            /* 31 */
  /* Return with a result returned on the stack */
  { "retr", 0, EFF_STUPIDEFF },         /* 32 */
  /* Displays the string register value (to current talk, sign, scroll or book),
    string register emptied */
  { "say", 0, EFF_STUPIDEFF },          /* 33 */
  { NULL, 0, 0 },           /* 34 */
  { NULL, 0, 0 },           /* 35 */
  { NULL, 0, 0 },           /* 36 */
  { NULL, 0, 0 },           /* 37 */
  /* Calls engine's intrinsic function with specified number of parameters 
    (popping them). The return value remains on stack */
  { "callis", 3, EFF_STUPIDEFF },          /* 38 */
  /* Calls engine's intrinsic function with specified number of parameters 
    (popping them). No return value  */
  { "calli", 3, EFF_SINGLELINE },         /* 39 */
  { NULL, 0, 0 },           /* 3a */
  { NULL, 0, 0 },           /* 3b */
  { NULL, 0, 0 },           /* 3c */
  { NULL, 0, 0 },           /* 3d */
  /* Pushes identifier of the item ( for which the usecode event handler is called ) 
    on the stack */
  { "push\titemref", 0, EFF_PUSH },        /* 3e */
  /* Aborts the function & all usecode execution, returning to the engine */
  { "exit", 0, EFF_SINGLELINE },         /* 3f */
  /* Removes all answers from the current talk */
  { "cla", 0, 0 },            /* 40 */
  { NULL, 0, 0 },           /* 41 */
  /* Pushes game flag's value (boolean) on the stack */
  { "pushf", 2, EFF_PUSH },         /* 42 */
  /* Pops the stack value to the game flag (boolean) */
  { "popf", 2, EFF_POP },          /* 43 */
  /* Pushes an immediate byte to the stack */
  { "pushbi", 1, EFF_PUSH },        /* 44 */
  { NULL, 0, 0 },           /* 45 */
  /* Uses the top-of-stack value to index (1-based) the array variable, (top-of-stack - 1)  as
    the new value, and updates a value in the array slot (local variable specified
    in the operand) */
  { "aput", 2, EFF_ARRAY | EFF_SINGLELINE },          /* 46 */
  /* Call of usecode function - function # is the operand
    ???Suggestion: functions are divided into event handlers (called by external code
    and the usecode) and functions (called by usecode only). "calle" is used to
    call the event handler from the usecode, while "call" is used to call a function
  */
  { "calle", 2, 0 },          /* 47 */
  /* Pushes the cause of usecode event handler call on the stack */
  /*  (double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG ) */
  { "push\teventid", 0, EFF_PUSH },        /* 48 */
  { NULL, 0, 0 },           /* 49 */
  /* Pops the value from the stack and adds it to array on the top of stack */
  /*  or pops both values from the stack, builds an array from them */
  /*  & pushes it on the stack if neither of them are arrays */
  { "arra", 0, EFF_STUPIDEFF },         /* 4a */
  /* Pops the top-of-stack value & sets the cause of usecode event handler to it
    (double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG )
    Used to call event handlers from the usecode
  */
  { "pop\teventid", 0, 0 },         /* 4b */
};

/* Embedded function table for Black Gate */
const char* bg_func_table[] =
{
  /* Random1(Hi) - Returns a random integer or from 1 to Hi inclusively
    Used to select a random party member for remarks
  */
  "Random1",            /* 0 */
  /* Takes itemref + array of several integers.
    Executes a macro for this itemref
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
  */
  "MacroExec",             /* 1 */
  /* Takes itemref + array of several integers + one more integer
    The same as above - but schedules a macro for execution later
    (use a serpent venom on somebody - first raises stats, then - after some time
      - lower them)
    Can also shout exclamations - "Call it... Heads... Its tails..."
      - double-click on money.
  */
  "MacroDelay",             /* 2 */
  /* SwitchTalkTo(itemref, face_frame) - switches conversation to other NPC,
    selecting one of possible face pictures
    Shows the face in the bottom of the screen if the face on top was not hidden yet
    Switching talk to -277 means - huge reg Guardian semi-transparent face
  */
  "SwitchTalkTo",           /* 3 */
  /* HideNPC(itemref) - hides NPC face away from the conversation */
  "HideNPC",            /* 4 */
  /* AddAnswer(str_or_array) - adds a answer or set of answers to */
  /*  the set of talk threads suggested */
  "AddAnswer",            /* 5 */
  /* RemoveAnswer(str) - removes a given string from set of talk threads suggested */
  "RemoveAnswer",           /* 6 */
  /* SaveTalkStartNew() - saves the conversation state & starts */
  /*  new blank set of talk answers suggested */
  "SaveTalkStartNew",         /* 7 */
  /* RestoreTalk() - restores set of answers suggested */
  /*  as was saved in SaveTalkStartNew() */
  "RestoreTalk",            /* 8 */
  /* EmptyTalk() - removes all talk answers
    Looks like the same as "cla" opcode
  */
  "EmptyTalk",              /* 9 */
  /* GetAnswer() - asks user to select one of the talk threads & returns it
    Practically the same functionality as "ask" opcode
  */
  "GetAnswer",            /* a */
  /* GetAnswerIndex() - asks user to select one of the talk threads
    & returns the 1-based index of it instead the text itself
  */
  "GetAnswerIndex",           /* b */
  /* AskNumber(min, max, step, default) - asks user a number by showing a slider */
  /*  with given parameters */
  "AskNumber",            /* c */
  /* SetItemType(itemref, type) - changes a type of given item to the new value */
  "SetItemType",            /* d */
  /* AreItemsNearNPC(NPCID, type, distance) - returns boolean whether there are 
    items of such Type at the given distance from the given NPC.
    Distance -1 means - screen borders
  */
  "AreItemsNearNPC",            /* e */
  /* PlaySoundEffect(sound_num) - plays a given effect soundtrack
  */
  "PlaySoundEffect",              /* f */
  /* Random2(lo, hi) - returns a random integer from lo to hi inclusively */
  "Random2",            /* 10 */
  /* GetItemType(itemref) - returns a "type" value of the item */
  "GetItemType",            /* 11 */
  /* GetItemFrame(itemref) - returns a "frame" value of the item */
  "GetItemFrame",           /* 12 */
  /* SetItemFrame(itemref, frame) - sets a "frame" value for the item */
  "SetItemFrame",           /* 13 */
  /* GetItemQuality(itemref) - returns a "quality" value of the item */
  "GetItemQuality",         /* 14 */
  /* SetItemQuality(itemref, quality) - sets a "quality" value for the item
    (Penumbra's plaque)
    Returns old quality???
  */
  "SetItemQuality",       /* 15 */
  NULL,             /* 16 */
  NULL,             /* 17 */
  /* GetItemCoords(itemref) - parameter is itemref returned by
      ItemSelectModal(), returns array
    The array is 2 item's coordinates - array[1], array[2]
  */
  "GetItemCoords",          /* 18 */
  NULL,             /* 19 */
  NULL,             /* 1a */
  /* GetNPCID(itemref) - returns NPCID from Itemref - possibly does-nothing function???
    Accepts values like -2 for Spark or -356 for Avatar on input
  */
  "GetNPCID",         /* 1b */
  /* GetNPCClass(NPCID) - returns a number specifying the NPC class -
    fighter/sage/shopkeeper etc. Used in exclamations like "Oh my aching back!"
    10 seems to be Noble (Finnegan & his aching back :-) )
    Or maybe this is GetNPCActivity???
  */
  "GetNPCClass",          /* 1c */
  /* SetNPCActivity(NPCID, activity) - sets an NPC activity
    11 - Loiter (leave with "go home")
    15 - Wait (leave with "wait here")
    ??? in cheat menu order?
  */
  "SetNPCActivity",             /* 1d */
  /* JoinNPC(itemref) - joins NPC to the party
    This seems to only set Party flag on - the same effect as doing this in cheat mode
    ???in BG, unusual companions (joined by cheat) will not be shown in Inventory/Ztats
      gumps. Where this information (whether the NPC is a usual companion) is kept?
    In SI, they will (I saw Rotoluncia's Ztats - though no 2 keys + a wand usually taken
      from her when dead in inventory. Trying to display Vasculio's Ztats hung
      the game) - but headless :-)
    Once more - it is kept somewhere...
  */
  "JoinNPC",              /* 1e */
  /* DismissNPC(itemref) - dismisses NPC from the party */
  "DismissNPC",             /* 1f */
  /* GetNPCStat(NPCID, property_id) -
    returns NPC's statistic value (9 is food level, 0 is Strength, 3 is Hits, )
    (1 & 4 seems to be Training & Combat???)
    Vas Mani (Restoration) spell is a good example
  */
  "GetNPCStat",         /* 20 */
  /* ChangeNPCStat(itemref, property_id, delta) - 
    increments (or decrements for negative delta) NPC's statistic value.
  */
  "ChangeNPCStat",          /* 21 */
  /* AvatarNPCID() - returns Avatar's NPC ID */
  "AvatarNPCID",            /* 22 */
  /* GetPartyMembers() - returns the array of NPCs which are party members currently */
  /*  ??? NPC IDs returned??? */
  "GetPartyMembers",          /* 23 */
  /* CreateItem(Type) - creates an item of given Type, returns itemref
    Must be followed by SetItemFrame()???
  */
  "CreateItem",           /* 24 */
  NULL,             /* 25 */
  NULL,             /* 26 */
  /* GetNPCName(npc_id_or_array) - returns an NPC name by NPC ID
    Returns an array of names if array was specified
  */
  "GetNPCName",           /* 27 */
  /* Counts the amount of party gold if called as Func28(-357, 644, -359, -359)
    644 is for sure the Type of gold heap
    649 is for sure the Type of serpent venom
    839 for hourglass
    Last parameter seems to be the frame number of the items - -359 means "any"
  */
  "CountNoItems",             /* 28 */
  /* Returns itemref of the given item in Avatar's (or party's??? - parameter 4???)
      posessions
    0 if no such item
    Func28(-357, 761, -359, -359) - spellbook.
    Func28(-357, 839, -359, 0) - Nicodemus's hourglass
    (known as Hourglass Of Fate in SI).
  */
  "ReturnItemsItemRef",             /* 29 */
  /* GetContainerItems(container_itemref, type, quality, ???) - returns an array */
  /*  ??? of itemrefs */
  "GetContainerItems",          /* 2a */
  /* Reduces the amount of party gold if called Func2b(delta, 644, -359, -359, true) 
    Reduces the amount of party eggs if called Func2b(delta, 377, -359, 24, true) 
  */
  "ReduceNoItems",             /* 2b */
  /* Increases the amount of party gold if called Func2c(delta, 644, -359, -359, true) 
    Returns false if party is overloaded & cannot accept this
  */
  "IncreaceNoItems",             /* 2c */
  /* Not used at all */
  NULL,             /* 2d */
  /* PlayMusic(song#, itemref) - plays a MIDI music track. */
  /*  Song# = 255 means - mute the MIDI channel. */
  "PlayMusic",            /* 2e */
  /* IsNPCInParty(npc_id) - returns true if given NPC is in party, */
  /*  otherwise returns false */
  "IsNPCInParty",           /* 2f */
  /* Takes a Type(???), returns an array of all visible items/NPCs of this type */
  NULL,             /* 30 */
  /* IsNPCxxx(itemref) - returns true if given NPC is ????? */
  NULL,             /* 31 */
  /* DisplaySign(gump#, array_of_strings) - displays a sign/plaque */
  /*  of given gump number & given text (array of strings or single string). */
  "DisplaySign",          /* 32 */
  /*  ItemSelectModal() - switches the engine to cross-cursor "use" mode.
    Returns an itemref of the item selected by user by single-click. 
    Does not return till user will single-click on something.
    The entity returned can be treated as array with coords at array[2], array[3]
  */
  "ItemSelectModal",          /* 33 */
  /* Not used at all */
  NULL,             /* 34 */
  /*
    ItemsNearItem(itemref, type, distance, ???) - returns an array of items
      of given Type which are closer then Distance to the item specified by Itemref
      Parameter 4 is possibly frame number to compare???
      Type of -1 means - all NPC??? (Fear spell)
    (Powder keg & cannon ball near the cannon)
  */
  "ItemsNearItem",              /* 35 */
  /* Takes an NPCID as a single parameter, seems to return the amount of free
    space in its inventory
  */
  NULL,             /* 36 */
  NULL,             /* 37 */
  /* GetTimeHour() - returns hour part of current game time - 0-23 */
  "GetTimeHour",            /* 38 */
  /* GetTimeMinute() - returns minute part of current game time - 0-59 */
  "GetTimeMinute",          /* 39 */
  /* GetItemRef(NPCID) - returns Itemref from NPCID - possibly does-nothing function???
  */
  "GetItemRef",         /* 3a */
  NULL,             /* 3b */
  /* GetNPCxxx(NPCID) - returns the same??? NPC mode as set in function 3d */
  NULL,             /* 3c */
  /* SetNPCxxx(NPCID, mode) - ???sets some NPC mode
    often called with mode=2 before entering combat
    ???alignment
  */
  NULL,             /* 3d */
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
  "TeleportItem",             /* 3e */
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
  "VanishNPC",              /* 3f */
  /* ItemSay(itemref, str) - displays a string on the screen near the specified item */
  "ItemSay",            /* 40 */
  NULL,             /* 41 */
  /* GetItemZCoord(itemref) - returns Z coordinate of the item */
  "GetItemZCoord",              /* 42 */
  /* SetItemZCoord(itemref, coord) - sets a Z coordinate for the item */
  "SetItemZCoord",              /* 43 */
  /* GetWeather() - returns current weather - 0-3
    3 seems to negate magic
  */
  "GetWeather",           /* 44 */
  /* SetWeather(weather) - sets new current weather - 0-3 */
  "SetWeather",           /* 45 */
  NULL,             /* 46 */
  /* SummonCreature(Type, boolean) - engine under Kal Bet Xen (Swarm), Kal Xen (???)
    an Kal Vas Xen (Summon).
    Summons a creature of given Type. Second parameter is true only for Kal Vas Xen
    Return value???
  */
  "SummonCreature",       /* 47 */
  /* Shows a map of Britannia. Double-click on the map, Vas Wis (Peer) spell or reading
    the Brommer's Britannia book
  */
  "ShowMap",            /* 48 */
  /* KillNPC(itemref) - kills the given NPC. Owen's suicide is this function alone.
    Plaque falling on LB's head - KillNPC, then VanishNPC.
    Balayna takes the vial from you & dies - this function alone
    Death bolt spell uses it too.
    These are all occurences.
  */
  "KillNPC",            /* 49 */
  /* Some kind of comparing 2 numbers (2 params) - returns boolean
    Used to compare somebody' Strength to item's Quality
  */
  "???",              /* 4a */
  /* SetNPCAttackMode(NPCID, mode) - sets a combat mode for the NPC
    Mode 7 is Fleeing - others are in men
    Fear spell, using dirty diapers on people
      (it is the same as Fear spell) - mode 7
    LB attacking in an LB cheat room - mode 0
  */
  "SetNPCAttackMode",       /* 4b */
  /* SetTargetNPCToAttack(attacker_NPCID, target_NPCID) - sets target NPC to attack
    ??? how it is called in a cheat menu?
    Usually called with Avatar as target NPC
  */
  "SetTargetNPCToAttack",     /* 4c */
  /* CloneNPC(NPCID) - clones an NPC - Clone spell
    Returns something - return value never used.
  */
  "CloneNPC",           /* 4d */
  /* Not used at all */
  NULL,             /* 4e */
  /* ShowCrystalBall(coords_array) - shows a crystal ball view of a given world point
  */
  "ShowCrystalBall",        /* 4f */
  /* ShowWizardEye(parm1, parm2) - shows a view for telescope or Wizard Eye spell
    For telescope - ShowWizardEye(10000, 1000)
    For Wizard Eye spell - ShowWizardEye(45, 200)
  */
  "ShowWizardEye",        /* 50 */
  /* ResurrectNPC(itemref) - resurrects an NPC. Itemref is a dead body's itemref
    Returns false is cannot resurrect - LB in this case says "Alas..." and then
      about burial.
    ???what is the cause of such LB's behaviour?
  */
  "ResurrectNPC",         /* 51 */
  /* AddSpellToBook(spellnum, 0, spellbook_itemref) - Adds a new spell to the spellbook
    Returns false if there was already such spell in the book
    Maybe calling with 1 (never called such in BG) will remove the spell from the book?
  */
  "AddSpellToBook",       /* 52 */
  /* ExecuteSprite(sprite, coordx, coordy, speedx, speedy, ???, ???)
    - executes an explosion-like sprite
    First parameter is sprite number in SPRITES.VGA
    speedx & speedy are non zero is the sprite floats away (smoke from smokebomb)
    Last parameters is usually -1 (not so for smokebomb)
  */
  "ExecuteSprite",          /* 53 */
  /* Powder keg burst??? */
  NULL,             /* 54 */
  /* DisplayBook(itemref) - Displays book or scroll
    Text will be displayed further by "say" opcode
  */
  "DisplayBook",              /* 55 */
  /* StopTime(???) - freezes all NPCs for some time. Parameter is duration???? 
    Type 6be object (Egg???) calls this with its Quality as a parameter on eventid 3
    Also called in Stop Time spell with the parameter 100
  */
  "StopTime",             /* 56 */
  /* CauseLight(duration) - Glimmer/Light/Great Light spell logic
    duration is 110 for Glimmer, 500 for Light, 5000 for Great Light
  */
  "CauseLight",             /* 57 */
  /* itemref as a parameter, returns some boolean... 
    ???NPC is on barge
    At least true return from this blocks Mark spell - even mantra is not shouted
  */
  "???",              /* 58 */
  /* CauseEarthquake(???) - causes an earthquake.
    Parameter is 40 for Armageddon spell & variable for Tremor spell
    Also called from func 85e (Forsythe jumping in the Well of Souls) - with parameter 15
  */
  "CauseEarthquake",            /* 59 */
  /* IsPlayerFemale() - returns 0 if male, 1 if female */
  "IsPlayerFemale",         /* 5a */
  /* CauseArmageddon() - all logic of Armageddon spell except
    shouting mantras, weather change, earthquake, & setting the game flag which
    affects Batlin's & LB's behaviour.
    Walks through the all NPC list & makes all of the Dead with Hits < 0
    Called also in LB's cheat room - "Busted, you thieving...."
  */
  "CauseArmageddon",              /* 5b */
  /* Sets NPC to some state - like VanishNPC() or KillNPC() */
  "???",              /* 5c */
  /* CauseBlackout() - darken the whole screen for some time */
  "CauseBlackout",            /* 5d */
  /* ArraySize(a) - returns number of elements in the array */
  "ArraySize",            /* 5e */
  /* Something for Mark spell??? */
  "???",              /* 5f */
  /* Called only once, something in a Recall spell
    Called after setting activity to 31 (Follow Avatar) to all party members
    and resetting flag 0x39 (what is it???)
    No return value, single parameter which is itemref
  */
  NULL,             /* 60 */
  /* Called only once, parameters are NPC strength, 0, 12, 3
    Return value exists but not used
  */
  NULL,             /* 61 */
  /* IsUnderTheRoof() - returns true if the party is under the roof.
    Sextant (twill) will not work under the roof.
    Also used in some place (???) where it causes a shout "Try it outside!"
  */
  "IsUnderTheRoof",       /* 62 */
  /* SetOrreryState(coords_array, status) - sets the state of the planets in Brion's orrery.
    Coords are usually ordinary orrery coords - array of 2 integers. (59c, b4c)
    status is a small integer which influences the state of the planets.
  */
  "SetOrreryState",           /* 63 */
  /* Not used at all */
  NULL,            /* 64 */
  NULL,            /* 65 */
  NULL,            /* 66 */
  NULL,            /* 67 */
  NULL,            /* 68 */
  /* Function 0x69 - GetSpeechTrack()
    Returns a speech track number previously set by SetSpeech()
  */
  "GetSpeechTrack",            /* 69 */
  NULL,            /* 60 */
  NULL,            /* 6a */
  NULL,            /* 6b */
  NULL,            /* 6c */
  NULL,            /* 6d */
  NULL,            /* 6e */
  /* Function 0x6f - DeleteItem(itemref)
    Deletes an item
  */
  "DeleteItem",            /* 6f */
  /* Function 0x75 - SetSpeech(speech_number)
    Remembers a speech track number, returns false if speech effects are disabled
    or no sound card
  */
  /* Function 0x75 - StartEndGame(boolean).
    TRUE is successful endgame (wand against the gate),
    FALSE is unsuccessful (passing through the gate) */
  /* Function 0x76 - FireCannon(cannon_itemref, fire_direction, ball_type, ???,
                  cannon_type, cannon_type).
    Fires a cannon in specified direction
  */
  /* Function 0x7e - PlaySpeech()
    Plays a speech track set by SetSpeech()
  */
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
  /* Function 0x89 - SetNPCFlag(itemref, flagno).
    Flagno 1 is Slept
    Flagno 8 is Poisoned
    Sets flag to true
    */
  /* Function 0x8a - ResetNPCFlag(itemref, flagno).
    Flagno 1 is Slept
    Flagno 8 is Poisoned
    Sets flag to false
    */
};
