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
 */

Resurrect 0x8FE ()
{
	var resurrectlist;
	var resurrectables;
	var resurrectablenames;
	var index;
	var max;
	var npc;
	var body;
	var itemindex;
	var name;
	var pos;
	var foodlevel;

	resurrectlist = getNearbyBodies();

	resurrectables = getJoinableNPCNumbers();
	resurrectables = [resurrectables, CANTRA];

	resurrectablenames = getJoinableNPCNames();
	resurrectablenames = [resurrectablenames, "Cantra"];

	var msg = "son";
	if (UI_is_pc_female())
		msg = "daughter";

	for (body in resurrectlist with index to max)
	{
		if (body != AVATAR->get_npc_object())
		{
			itemindex = getIndexForElement(npc, resurrectables);
			name = resurrectablenames[itemindex];
			var flag_dont_resurrect = false;
			npc = body->get_body_npc();
			if (gflags[BANES_RELEASED])
			{
				// Prevent "resurrection" of the three Banes:
				if ((npc == DUPRE) && !gflags[WANTONESS_BANE_DEAD])
					flag_dont_resurrect = true;
				else if ((npc == IOLO) && !gflags[INSANITY_BANE_DEAD])
					flag_dont_resurrect = true;
				else if ((npc == SHAMINO) && !gflags[ANARCHY_BANE_DEAD])
					flag_dont_resurrect = true;

				if (flag_dont_resurrect)
					say("@I am sorry, my ", msg, ", but ", name,
						"'s body vanished when I tried to raise it! It seems that it was but a cruel of illusion...@");
			}

			if (!flag_dont_resurrect && (npc == DUPRE) &&
			    gflags[DUPRE_IS_TOAST] && gflags[WANTONESS_BANE_DEAD])
			{
				// Prevent resurrection of Dupre after the Crematorium:
				flag_dont_resurrect = true;
				say("@I am sorry, my ", msg,
					", but I cannot -- this isn't Dupre's body, but merely shadow of him. See how it diappears when I try to rause it...@");
			}

			var pos = npc->get_object_position();
			if (!flag_dont_resurrect && !npc->is_dead() &&
			    // Check to see if the NPC is *outside* of the
			    // House of the Dead:
			    (!((pos[X] >= 0x40) && (pos[Y] >= 0) &&
			    (pos[X] <= 0xEF) && (pos[Y] <= 0x4F))))
			{
				// Prevent resurrection of a live NPC:
				flag_dont_resurrect = true;
				say("@I know not who this is, but it isn't thy friend -- he is still alive somewhere. The similarity is remarkable, though... But look! The body disapeared! I wonder what has happened to it?@");
			}

			if (flag_dont_resurrect)
				body->remove_item();

			else
			{
				itemindex = getIndexForElement(npc, resurrectables);
				name = resurrectablenames[itemindex];
				pos = npc->get_object_position();

				if (body->UI_resurrect())
				{
					say("@Now thy friend ", name, " doth live again.@");
					if (npc->get_item_flag(IN_PARTY) &&
					    npc->get_item_flag(SI_ZOMBIE))
						npc->remove_from_party();

					npc->set_new_schedules(MIDNIGHT, WAIT, [pos[X], pos[Y]]);
					npc->run_schedule();
					npc->set_schedule_type(WAIT);

					foodlevel = getPropValue(npc, FOODLEVEL);
					foodlevel = (31 - foodlevel);
					setPropValue(npc, FOODLEVEL, foodlevel);

					if (npc == GWENNO)
						gflags[GWENNO_IS_DEAD] = false;
				}
				else
					say("@Thy friend ", name, " hath been lost forever.@");
			}
		}
	}
}
