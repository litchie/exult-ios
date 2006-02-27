/*
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *	This header file overrides the spellbook's spells (most of them, anyway)
 *	to use the new versions. I preferred to do it this way for four reasons:
 *		(1) There is a clean algorythm by which NPC spells are cast;
 *		(2) It allows me to override specific spells and leave others;
 *		(3)	I can add new NPC-only spells without removing existing spells.
 *		(4) It is easier to add NPC-specific spells which only one NPC
 *			can cast (although I haven't made any such spells yet).
 *
 *	Presently, I haven't overriden Help (except for Multimap support), Recall
 *	and Armageddon -- I don't think that they are appropriate for NPCs, so I
 *	left them alone. I _have_ overriden Mark, but only so that is won't be
 *	possible to use it while inside the Shrine of the Codex.
 *	All other spells are overriden so that the Avatar and the NPCs play by the
 *	same rules.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

spellAwaken_Override			0x640 () {spellAwaken(UI_click_on_item());}
spellWeather_Override			0x641 () {spellWeather();}
spellDouse_Override				0x642 () {spellDouse(UI_click_on_item());}
spellFireworks_Override			0x643 () {spellFireworks();}
spellGlimmer_Override			0x644 () {spellGlimmer();}
spellHelp_Override				0x645 ()
{
	if (event == DOUBLECLICK)
		//If we came here from a doubleclick, forward to original:
		spellHelp_Override.original();
	else if (event == SCRIPTED)
	{
		//We got here from a Usecode script, so we override it
		//for multi-map support:
		var pos = [0x03A8, 0x047A, 0x0, 0x0];
		PARTY->move_object(pos);
	}
}
spellIgnite_Override			0x646 () {spellIgnite(UI_click_on_item());}
spellThunder_Override			0x647 () {spellThunder();}

spellCreateFood_Override		0x648 () {spellCreateFood();}
spellCure_Override				0x649 () {spellCure(UI_click_on_item());}
spellDetectTrap_Override		0x64A () {spellDetectTrap();}
spellGreatDouse_Override		0x64B () {spellGreatDouse();}
spellGreatIgnite_Override		0x64C () {spellGreatIgnite();}
spellLight_Override				0x64D () {spellLight();}
spellLocate_Override			0x64E () {spellLocate();}
spellAwakenAll_Override			0x64F () {spellAwakenAll();}

spellDestroyTrap_Override		0x650 () {spellDestroyTrap(UI_click_on_item());}
spellEnchant_Override			0x651 () {spellEnchant(UI_click_on_item());}
spellFireBlast_Override			0x652 () {spellFireBlast(UI_click_on_item());}
spellGreatLight_Override		0x653 () {spellGreatLight();}
spellMassCure_Override			0x654 () {spellMassCure();}
spellProtection_Override		0x655 () {spellProtection(UI_click_on_item());}
spellTelekinesis_Override		0x656 () {spellTelekinesis(UI_click_on_item());}
spellWizardEye_Override			0x657 () {spellWizardEye();}

spellCurse_Override				0x658 () {spellCurse(UI_click_on_item());}
spellHeal_Override				0x659 () {spellHeal(UI_click_on_item());}
spellSwarm_Override				0x65A () {spellSwarm();}
spellProtectAll_Override		0x65B () {spellProtectAll();}
spellParalyze_Override			0x65C () {spellParalyze(UI_click_on_item());}
spellPeer_Override				0x65D () {spellPeer();}
spellPoison_Override			0x65E () {spellPoison(UI_click_on_item());}
spellSleep_Override				0x65F () {spellSleep(UI_click_on_item());}

spellConjure_Override			0x660 () {spellConjure();}
spellLightning_Override			0x661 () {spellLightning(UI_click_on_item());}
spellMark_Override				0x662 ()
{
	var target;
	var pos;
	var in_codex_shrine;

	if (event == DOUBLECLICK)
	{
		//Stop Avatar's usecode:
		halt_scheduled();
		//Prompt for a target:
		target = UI_click_on_item();
		//For Codex Quest: of the Virtue Stone of Spirituality has been
		//reattuned to the Shrine of Spirituality, clear the flag if the
		//player decides to cast Mark on it. This allows the Codex to restore
		//the stone once again if needed.
		if ((target->get_item_frame() == 7) && gflags[ATTUNED_SPIRITUALITY_STONE])
			gflags[ATTUNED_SPIRITUALITY_STONE] = false;
		
		if (target->get_map_num() == 0)
		{
			//Get the stone's position:
			pos = target->get_object_position();
	
			//Check to see if the STONE is inside the area of the Shrine of the Codex: 
			if (!UI_is_pc_inside() && (pos[X] >= 0xA50) && (pos[Y] >= 0xABC) && (pos[X] <= 0xAE0) && (pos[Y] <= 0xB1D))
				in_codex_shrine = true;
			if ((pos[X] >= 0xA80) && (pos[Y] >= 0xADC) && (pos[X] <= 0xABB) && (pos[Y] <= 0xB0D))
				in_codex_shrine = true;
		}
		
		if (!in_codex_shrine)
		{
			//The stone is not inside the area guarded by the Guardians;
			//forward to original usecode:
			//Whoever invented this intrinsic deserves commendation:
			target->set_intercept_item();
			spellMark_Override.original();
		}
		else
		{
			//The stone is in the forbidden zone, so the spell fails:
			item_say("@Kal Por Ylem@");
			script item
			{	actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}
spellMassCurse_Override			0x663 () {spellMassCurse();}
//spellRecall_Override			0x667 () {spellRecall_Override.original();}
spellReveal_Override			0x665 () {spellReveal();}
spellSeance_Override			0x666 () {spellSeance();}
spellUnlockMagic_Override		0x667 () {spellUnlockMagic(UI_click_on_item());}

spellCharm_Override				0x668 () {spellCharm(UI_click_on_item());}
spellDance_Override				0x669 () {spellDance();}
spellDispelField_Override		0x66A () {spellDispelField(UI_click_on_item());}
spellExplosion_Override			0x66B () {spellExplosion(UI_click_on_item());}
spellGreatHeal_Override			0x66C () {spellGreatHeal(UI_click_on_item());}
spellInvisibility_Override		0x66D () {spellInvisibility(UI_click_on_item());}
spellFireField_Override			0x66E () {spellFireField(UI_click_on_item());}
spellMassSleep_Override			0x66F () {spellMassSleep();}

spellCauseFear_Override			0x670 () {spellCauseFear();}
spellClone_Override				0x671 () {spellClone(UI_click_on_item());}
spellFireRing_Override			0x672 () {spellFireRing(UI_click_on_item());}
spellFlameStrike_Override		0x673 () {spellFlameStrike();}
spellMagicStorm_Override		0x674 () {spellMagicStorm();}
spellPoisonField_Override		0x675 () {spellPoisonField(UI_click_on_item());}
spellSleepField_Override		0x676 () {spellSleepField(UI_click_on_item());}
spellTremor_Override			0x677 () {spellTremor();}

spellCreateGold_Override		0x678 () {spellCreateGold(UI_click_on_item());}
spellDeathBolt_Override			0x679 () {spellDeathBolt(UI_click_on_item());}
spellDelayedBlast_Override		0x67A () {spellDelayedBlast(UI_click_on_item());}
spellEnergyField_Override		0x67B () {spellEnergyField(UI_click_on_item());}
spellEnergyMist_Override		0x67C () {spellEnergyMist(UI_click_on_item());}
spellMassCharm_Override			0x67D () {spellMassCharm();}
spellMassMight_Override			0x67E () {spellMassMight();}
spellRestoration_Override		0x67F () {spellRestoration();}

//spellArmaggedon_Override		0x680 () {spellArmaggedon_Override.original();}
spellDeathVortex_Override		0x681 () {spellDeathVortex(UI_click_on_item());}
spellMassDeath_Override			0x682 () {spellMassDeath();}
spellInvisibilityAll_Override	0x683 () {spellInvisibilityAll();}
spellResurrect_Override			0x684 () {spellResurrect(UI_click_on_item());}
spellSummon_Override			0x685 () {spellSummon();}
spellSwordStrike_Override		0x686 () {spellSwordStrike(UI_click_on_item());}
spellTimeStop_Override			0x687 () {spellTimeStop();}
