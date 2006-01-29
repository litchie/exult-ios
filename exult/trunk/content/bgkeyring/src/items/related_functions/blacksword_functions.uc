/*
 *	This source file contains some functions used by the blacksword.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

var isWorthyToKill 0x848 (var target_shape)
{
	var killables = [SHAPE_DRAGON, SHAPE_DRAKE, SHAPE_ETHEREAL_MONSTER, SHAPE_HYDRA, SHAPE_UNICORN,
					 SHAPE_FAIRY, SHAPE_WISP, SHAPE_MONSTER_CYCLOPS, SHAPE_CYCLOPS, SHAPE_DRAXINUSOM,
					 SHAPE_IOLO, SHAPE_SHAMINO, SHAPE_DUPRE, SHAPE_SPARK, SHAPE_HOOK,
					 SHAPE_MAGE_MALE, SHAPE_MAGE_FEMALE, SHAPE_MONSTER_MAGE];
	
	if (target_shape in killables) return true;
	else return false;
}

var isUndead 0x849 (var target_shape)
{
	var undead = [SHAPE_LICHE2, SHAPE_LICHE, SHAPE_SKELETON, SHAPE_GHOST2, SHAPE_GHOST_MALE, SHAPE_GHOST_FEMALE];

	if (target_shape in undead) return true;
	else return false;
}

teleportIsleOfFire 0x6F9 ()
{
	if (event == SCRIPTED)
	{
		if (get_item_shape() != SHAPE_BLACKSWORD)
			//Forward to original:
			teleportIsleOfFire.original();
		else
		{
			//Modified this part simply for multimap compatibility:
			var dest = [0x890, 0x5FA, 0x0, 0x0];
			if (!UI_get_item_flag(AVATAR, ON_MOVING_BARGE))
			{
				UI_fade_palette(12, 1, 0);
				PARTY->move_object(dest);
				script AVATAR
				{	face NORTH;				wait 1;
					call teleportIsleOfFire;}
			}
		}
	}
	else
		//Forward to original:
		teleportIsleOfFire.original();
}