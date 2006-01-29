/*
 *	This source file contains some functions specific to a cutscene
 *	from the Keyring Quest.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Mage_male shape#(445) ()
{
	if ((event == DEATH) &&  (get_npc_id() == ID_MAGE_OR_GOON))
	{
		//Laundo has died
		//Set flag:
		gflags[MAGE_KILLED] = true;
		//Halt the fireworks around Laurianna:
		LAURIANNA->halt_scheduled();
		//Prepare quest advancement:
		registerDeathOfMageOrGoon();
	}
	else
		Mage_male.original();
}

Gargoyle_warrior shape#(274) ()
{
	if ((event == DEATH) && (get_npc_id() == ID_MAGE_OR_GOON))
		//Agra-Lem has died
		//Prepare quest advancement:
		registerDeathOfMageOrGoon();
	else
		Gargoyle_warrior.original();
}

Cyclops shape#(380) ()
{
	if ((event == DEATH) && (get_npc_id() == ID_MAGE_OR_GOON))
		//Laundo's cycloptic goon has died
		//Prepare quest advancement:
		registerDeathOfMageOrGoon();
	else
		Cyclops.original();
}

Troll shape#(861) ()
{
	if ((event == DEATH) && (get_npc_id() == ID_MAGE_OR_GOON))
		//Laundo's troll goon has died
		//Prepare quest advancement:
		registerDeathOfMageOrGoon();
	else
		Troll.original();
}

Fighter_male shape#(462) ()
{
	if ((event == DEATH) && (get_npc_id() == ID_MAGE_OR_GOON))
		//Laundo's fighter goon has died
		//Prepare quest advancement:
		registerDeathOfMageOrGoon();
	else
		Fighter_male.original();
}
