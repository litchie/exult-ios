/*
 *	This source file contains some spellcasting related constants, as well as
 *	a few general-purpose spell functions. IT IS NOT FINISHED YET!
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

const int SCRIPT_FACE_DIR			= 0x59;
const int SCRIPT_NPC_FRAME			= 0x61;

//Cool new spell stuff:
enum npc_wizard_flags
{
	//New spell effects:
	CONFUSION						= 40,	//Under the effects of Consufion spell
	//Spellcasting power:
	MAGE_CLASS						= 50,	//Full spellcasting capabilities
	BARD_CLASS						= 51,	//Half-level spellcasting
	//The next constants are used to control spellcaster AI; the AI is NOT
	//IMPLEMENTED YET.
	WIZ_HEALER						= 56,
	WIZ_KINETICIST					= 57,
	WIZ_DEATH_MAGE					= 58,
	WIZ_ENCHANTER					= 59,
	WIZ_WARMAGE						= 60,
	WIZ_CONJURER					= 61,
	WIZ_DIVINER						= 62,
	WIZ_THAUMATURGE					= 63
};

greatDouseIgnite 0xB00 (var shapes)
{
	var dist = 25;
	var index;
	var max;
	var obj;
	
	for (obj in shapes with index to max)
	{
		//For each shape # in the array,
		var index2;
		var max2;
		var lightsource;
		//Find all nearby objects with that shape...
		var lsources = find_nearby(obj, dist, MASK_NONE);
		for (lightsource in lsources with index2 to max2)
		{
			//For each object in the array,
			//calculate a delay based on distance:
			var delay = ((get_distance(lightsource) / 3) + 2);
			//Call the obj's usecode function after delay ticks:
			script lightsource after delay ticks
			{	nohalt;					call lightsource->get_usecode_fun(), DOUBLECLICK;}
		}
	}
}

var getFriendlyTargetList (var caster, var dist)
{
	if ((caster->get_npc_object()) in UI_get_party_list())
		//Caster is in the party; return party:
		return UI_get_party_list();
	else
	{
		//Caster not in party:
		var npc;
		var index;
		var max;
		//Get all nearby NPCs:
		var npclist = getNearbyNonPartyNPCs(dist);
		var retlist = [];
		//Get caster's alignment:
		var align = caster->get_alignment();
		for (npc in npclist with index to max)
			//For each NPC in the list,
			if (align == npc->get_alignment())
				//Add NPC to list if align matches caster's:
				retlist = [retlist, npc];
		if (align == 0)
			//Add party to list if caster is friendly:
			retlist = [retlist, UI_get_party_list()];
		return retlist;
	}
}

var getEnemyTargetList (var caster, var dist)
{
	if ((caster->get_npc_object()) in UI_get_party_list())
		//If caster in party, return all non-party NPCs:
		return getNearbyNonPartyNPCs(dist);
	else
	{
		//Caster not in party:
		var npc;
		var index;
		var max;
		//Get all nearby NPCs:
		var npclist = getNearbyNonPartyNPCs(dist);
		var retlist = [];
		//Get caster's alignment:
		var align = caster->get_alignment();
		for (npc in npclist with index to max)
			//For each NPC in the list,
			if (align != npc->get_alignment())
				//Add NPC to list if align doesn't match caster's:
				retlist = [retlist, npc];
		if (align != 0)
			//Add party to list if caster is not friendly:
			retlist = [retlist, UI_get_party_list()];
		return retlist;
	}
}

DeathBoltHit 0xB7F ()
{
	//Bail out unless an NPC is hit:
	if (is_npc())
	{
		//Get NPC's intelligence:
		var targetint = get_npc_prop(INTELLIGENCE);
		//See if he can't die:
		var cantdie = get_item_flag(CANT_DIE);
		//Opposed roll to see if we have a corpse in hands:
		if (UI_roll_to_win(20, targetint) && !cantdie)
			script item hit 127;
	}
}
