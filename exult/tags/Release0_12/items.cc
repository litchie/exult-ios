/**
 **	Items.cc - Names of items.
 **
 **	Written: 11/5/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <fstream.h>
#include "items.h"
#include "utils.h"

char *item_names[1024];			// Names of U7 items.

/*
 *	Set up names of items.
 */

/*
void Setup_item_names
	(
	)
	{
	for (int i = 0; i < sizeof(item_names)/sizeof(item_names[0]); i++)
		item_names[i] = 0;
	item_names[MAGIC_HELM] = "magic helm";
	item_names[GREAT_HELM] = "great helm";
	item_names[BUCKLER] = "buckler";
	item_names[SWAMP_BOOT] = "swamp boots";
	item_names[MAGIC_SHIELD] = "magic shield";
	item_names[SPIKE_SHIELD] = "spike shield";
	item_names[MAGIC_ARMOR] = "magic armor";
	item_names[MAGIC_LEGGINGS] = "magic leggings";
	item_names[MAGIC_GAUNLETS] = "magic gaunlets";
	item_names[MAGIC_GORGET] = "magic gorget";
	item_names[REGENERATION] = "regeneration";
	item_names[PROTECTION] = "protection";
	item_names[INVISIBILITY] = "invisibility";

	item_names[GLASS_SWORD] = "glass sword";
	item_names[MAGEBANE_SWORD] = "magebane sword";
	item_names[FIRE_SWORD] = "fire sword";
	item_names[MAGIC_SWORD] = "magic sword";
	item_names[DEFENSE_SWORD] = "defense sword";
	item_names[CUSTOM_SWORD] = "custom sword";
	item_names[SERPENT_SWORD] = "serpent sword";
	item_names[GREAT_DAGGER] = "great dagger";
	item_names[POISON_DAGGER] = "poison dagger";
	item_names[SERPENT_DAGGER] = "serpent dagger";
	item_names[HOE_OF_DESTRUCTION] = "hoe of destruction";
	item_names[THE_DEATH_SCYTHE] = "the death scythe";
	item_names[LIGHTNING_WHIP] = "lightning whip";
	item_names[MUSKET] = "musket";
	item_names[MUSKET_AMMO] = "musket ammo";
	item_names[SLING] = "sling";
	item_names[MAGIC_BOOMERANG] = "magic boomerang";
	item_names[MAGIC_AXE] = "magic axe";
	item_names[JUGGERNAUT_HAMMER] = "juggernaut hammer";
	item_names[MAGIC_BOW] = "magic bow";
	item_names[BURST_ARROW] = "burst arrow";
	item_names[MAGIC_ARROW] = "magic arrow";
	item_names[LUCKY_ARROW] = "lucky arrow";
	item_names[LOVE_ARROW] = "love arrow";
	item_names[TSERAMED_ARROW] = "tseramed arrow";
	item_names[BLOW_GUN] = "blow gun";
	item_names[STARBURST] = "starburst";
	item_names[CROSSBOW] = "crossbow";
	item_names[WHIP] = "whip";
	item_names[TRIPLE_CROSSBOW] = "triple crossbow";
	item_names[FIREDOOM_STAFF] = "firedoom staff";
	item_names[LIGHTNING_WAND] = "lightning wand";
	item_names[FIRE_WAND] = "fire wand";
	item_names[MAGICIANS_WAND] = "magicians wand";

	item_names[GOLD_COIN] = "gold coin";
	item_names[GOLD_NUGGET] = "gold nugget";
	item_names[GOLD_BAR] = "gold bar";
	item_names[GEMS] = "gem";
	item_names[FOOD] = "food";
	item_names[WINE] = "wine";
	item_names[HONEY] = "honey";
	item_names[MAGIC_POTIONS] = "magic potion";
	item_names[LIGHT] = "light";
	item_names[TORCH] = "torch";
	item_names[VIRTUE_STONES] = "virtue stone";
	item_names[RUNES] = "runes";
	item_names[ORB_OF_MOONS] = "orb of moons";
	item_names[SPACE_SHIP] = "space ship";
	item_names[CLOCK] = "clock";
	item_names[POCKET_WATCH] = "pocket watch";
	item_names[SEXTANT] = "sextant";
	item_names[BEDROLL] = "bedroll";
	item_names[HAMMER] = "hammer";
	item_names[LOCKPICK] = "lockpick";
	item_names[CADDELLITE_HELMET] = "caddellite helmet";
	item_names[CADDELLITE_CHUNK] = "caddellite chunk";
	item_names[RING] = "ring";
	item_names[KEYS] = "key";
	item_names[BOOKS] = "book";
	item_names[SCROLLS] = "scroll";
	item_names[SLEEPING_POWDER] = "sleeping powder";
	item_names[INVISIBILITY_DUST] = "invisibility dust";
	item_names[SERPENT_VENOM] = "serpent venom";
	item_names[CANNON] = "cannon";
	item_names[CANNON_BALL] = "cannon ball";
	item_names[GUN_POWDER_KEG] = "gun powder keg";
	item_names[ORRERY_CRYSTAL] = "orrery crystal";
	item_names[ORRERY_VIEWER] = "orrery viewer";
	item_names[SOUL_CAGE] = "soul cage";
	item_names[MUSIC_BOX] = "music box";
	item_names[CALTROPS] = "caltrops";
	item_names[SPELL_BOOK] = "spell book";
	item_names[SPELL_COMPONENTS] = "spell components";
	item_names[SMOKE_BOMB] = "smoke bomb";
	item_names[RUDYOMS_WAND] = "rudyoms wand";
	item_names[BLACK_ROCK] = "black rock";
	item_names[FLAME_OIL] = "flame oil";
	item_names[HOURGLASS] = "hourglass";
	item_names[ETHERAL_RING] = "etheral ring";
	item_names[GENERATORS] = "generator";
	item_names[WELL] = "well";
	item_names[ROULETTE] = "roulette";
	item_names[CLOSED_DOOR] = "door";
	item_names[OPEN_DOOR] = "door";
	item_names[CLOSED_HORIZ_SHUTTERS] = item_names[CLOSED_VERT_SHUTTERS] =
	item_names[OPEN_VERT_SHUTTERS] = item_names[OPEN_HORIZ_SHUTTERS] =
				"shutters";
	item_names[RAKE] = "rake";
	item_names[DEAD_BODY] = "dead body";
	item_names[TREE] = "tree";
	item_names[TABLE] = "table";
	item_names[VERT_BED] = "bed";
	item_names[HORIZ_BED]= "bed";
	item_names[STREETLAMP] = "street lamp";
	item_names[HUNG_GARGOYLE] = "dead gargoyle";
	item_names[WATER_TROUGH] = "water trough";
	item_names[CANDLESTICK] = "candlestick";
	item_names[CHEST] = "chest";
	item_names[POTTED_PLANT] = "potted plant";
	item_names[STONE_WALL] = "stone wall";
	item_names[POST] = "post";
	item_names[HORIZ_FENCE] = item_names[VERT_FENCE] = "fence";
	item_names[FOUNTAIN] = "fountain";
	}
*/

void Setup_item_names (ifstream& items) {
	items.seekg(0x54);
	int num_items = Read4(items);
	for(int i=0; i<1024; i++) {
		items.seekg(0x80+i*8);
		int itemoffs = Read4(items);
		if(!itemoffs)
			continue;
		int itemlen = Read4(items);
		items.seekg(itemoffs);
		item_names[i] = new char[itemlen];
		items.read(item_names[i], itemlen);
	}
} 
