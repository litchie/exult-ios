/*
 *	This source file contains some functions specific to a part
 *	of the Keyring Quest.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Liche shape#(354) ()
{
	if ((get_npc_id() == ID_JONELETH) && (event == DEATH))
	{
		//Joneleth the liche has died
		//Remove him from tournament mode:
		clear_item_flag(SI_TOURNAMENT);
		//Kill him:
		script item hit 50;
		//Start loop which will end when the Avatar picks
		//up at least one Gem of Dispelling:
		script AVATAR after 10 ticks
		{	nohalt;						call deleteLicheEggs;}
		abort;
	}
	else
		Liche.original();
}
