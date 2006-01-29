#include "headers/constants.uc";					//standard constant definitions
#include "headers/constants2.uc";					//standard constant definitions
#include "quests/keyring/constants.uc";				//The constants for the keyring quest

#include "headers/bg/bg_npcs.uc";					//Black Gate npc constants

#include "headers/bg/bg_shapes.uc";					//Black Gate shape and frame constants
#include "headers/bg/bg_shapes2.uc";				//Black Gate shape and frame constants
#include "headers/new_shapes.uc";					//Brand new shape and frame constants

#include "headers/bg/bg_gflags.uc";					//Black Gate global flags
#include "headers/bg/bg_gflags2.uc";				//Black Gate global flags
#include "headers/new_flags.uc";					//Brand new global flags

#include "headers/bg/bg_externals.uc";				//extern declarations for BG functions
#include "headers/bg/bg_externals2.uc";				//extern declarations for BG functions

#include "headers/new_items.uc";					//extern declarations for new items

#include "headers/functions.uc";					//new general-purpose functions

#include "headers/utility_functions.uc";			//Some new general-purpose functions
#include "quests/keyring/functions.uc";				//Functions for the keyring quest

//Functions related to items:
#include "items/related_functions/keyring_functions.uc";		//General key and keyring functions
#include "items/related_functions/arcadion_dialog.uc";			//Arcadion's dialog (broken into three functions) and related functions
#include "items/related_functions/blackrock_potion_kill.uc";	//The Blackrock potion's kill routine

//Cutscenes of the Keyring Quest:
#include "quests/keyring/cutscenes/zauriel_make_potion.uc";		//Zauriel makes the Blackrock potion
#include "quests/keyring/cutscenes/mage_and_goons.uc";			//The cutscene with Laundo and his goons
#include "quests/keyring/cutscenes/zauriel_ritual.uc";			//Zauriel performs the curing ritual
#include "quests/keyring/cutscenes/cheaters_cataclysm.uc";		//The cataclysm triggered by hackmocing Laurianna with the amulet near to her father

#include "misc/show_codex.uc";									//Displays the Codex

#include "npcs/related_functions/zauriel_functions.uc";			//Misc Zauriel functions
#include "npcs/related_functions/zauriel_dialog.uc";			//Zauriel's dialog functions
#include "npcs/related_functions/laurianna_functions.uc";		//Misc Laurianna functions
#include "npcs/related_functions/laurianna_dialog.uc";			//Laurianna's dialog functions

/*
 *	Starting here, all functions have predefined numbers. This can be for
 *	any of the reasons below:
 *		1) They are reimplementations of standard BG functions;
 *		2) They are whole new functions used for eggs;
 *		3) They are item functions;
 *		4) They are NPC functions.
*/

#include "items/related_functions/blacksword_functions.uc";		//Reimplemented blacksword functions

//Egg functions:
#include "misc/codex_eggs.uc";						//Eggs for displaying the Codex
#include "quests/keyring/eggs.uc";					//The keyring quest egg functions
#include "quests/locklake/cleanup_eggs.uc";			//Eggs that gradually clean Lock Lake
#include "misc/inn_key_eggs.uc";					//Deletes inn keys, tidies up beds and locks inn doors

//Death usecode for the mage and his goons
#include "quests/keyring/cutscenes/related_functions/mage_and_goons_death.uc";
//Death usecode for Joneleth the liche
#include "quests/keyring/liche_death.uc";

//Spellbook override:
#include "spells/spellbook_override.uc";			//Makes avatar use the new spells

//Item functions:
#include "items/orb_of_the_moons.uc";				//New Orb of the Moons
#include "items/eternal_flames.uc";					//Flames of Principle, Infinity and Singularity
#include "items/plaque.uc";							//Flames of Principle, Infinity and Singularity
#include "items/rings.uc";							//Fixes rings so that they must be on your finger to work
#include "items/keyring.uc";						//The keyring function
#include "items/key.uc";							//The key function
#include "items/zauriel_journal.uc";				//Zauriel's journal
#include "items/blacksword.uc";						//New blacksword function
#include "items/gem_of_dispelling.uc";				//For the keyring quest
#include "items/blackrock_potion.uc";				//For the keyring quest
#include "items/lens.uc";							//For viewing the Codex
#include "items/magic_carpet.uc";					//Prevents landing he carpet near the Codex Shrine
#include "items/shrines.uc";						//For meditation at shrines
#include "items/remote_viewers.uc";					//Multimap support of orrery viewer, Isle of Fire gem and crystal balls

//NPCs:
#include "npcs/wisps.uc";							//Modified wisps that are compatible with the new orb
#include "npcs/lordheather.uc";						//Kick start Lock Lake cleaning by signing bill
#include "npcs/perrin.uc";							//Modified for compatibility with Keyring quest
#include "npcs/reyna.uc";							//Modified for compatibility with Keyring quest
#include "npcs/zauriel.uc";							//Keyring quest NPC
#include "npcs/laurianna.uc";						//Keyring quest NPC
