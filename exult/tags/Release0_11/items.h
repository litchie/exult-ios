/**
 **	Items.h - Names of items.
 **
 **	Written: 12/3/98 - JSF
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

#ifndef INCL_ITEMS
#define INCL_ITEMS 1
#include <fstream.h>

extern char *item_names[1024];		// The game items' names.
//void Setup_item_names();		// Initialize.
void Setup_item_names (ifstream& items);

// Armor
#define MAGIC_HELM      383
#define GREAT_HELM	541
#define CRESTED_HELM	542
#define BUCKLER		543
#define SWAMP_BOOT      588
#define MAGIC_SHIELD    663
#define SPIKE_SHIELD    578
#define MAGIC_ARMOR     666
#define MAGIC_LEGGINGS  686
#define MAGIC_GAUNLETS  835
#define MAGIC_GORGET    843

// Magic rings
#define REGENERATION	 298
#define PROTECTION	297
#define INVISIBILITY	396

// Weapons
#define GLASS_SWORD         604
#define MAGEBANE_SWORD      559
#define FIRE_SWORD          551
#define MAGIC_SWORD         547
#define DEFENSE_SWORD       567
#define CUSTOM_SWORD        635
#define SERPENT_SWORD       637
#define GREAT_DAGGER        561
#define POISON_DAGGER       564
#define SERPENT_DAGGER      636
#define HOE_OF_DESTRUCTION  548
#define THE_DEATH_SCYTHE    562
#define LIGHTNING_WHIP      549
#define MUSKET              278
#define MUSKET_AMMO         581
#define SLING              474
#define MAGIC_BOOMERANG     550
#define MAGIC_AXE           552
#define JUGGERNAUT_HAMMER   557
#define MAGIC_BOW           606
#define BURST_ARROW         554
#define MAGIC_ARROW         556
#define LUCKY_ARROW         558
#define LOVE_ARROW          560
#define TSERAMED_ARROW      568
#define BLOW_GUN            563
#define STARBURST          565
#define CROSSBOW           598
#define WHIP		622
#define TRIPLE_CROSSBOW     647
#define FIREDOOM_STAFF      553
#define LIGHTNING_WAND      629
#define FIRE_WAND           630
#define MAGICIANS_WAND     792

// Misc
#define CHICKEN_COOP	  210
#define GOLD_COIN          644
#define GOLD_NUGGET        645
#define GOLD_BAR           646
#define GEMS              760
#define FOOD              377
#define WINE              616
#define HONEY             772
#define MAGIC_POTIONS      340
#define LIGHT             338
#define TORCH             595
#define VIRTUE_STONES      330
#define RUNES             877
#define ORB_OF_MOONS       785
#define SPACE_SHIP         267
#define CLOCK             252
#define POCKET_WATCH       159
#define SEXTANT           650
#define BEDROLL           583
#define HAMMER            623
#define LOCKPICK          627
#define CADDELLITE_HELMET  638
#define CADDELLITE_CHUNK   728
#define RING              640
#define KEYS              641
#define BOOKS             642
#define SCROLLS           797
#define SLEEPING_POWDER    648
#define INVISIBILITY_DUST  790
#define SERPENT_VENOM      649
#define CANNON            702
#define CANNON_BALL        703
#define GUN_POWDER_KEG     704
#define ORRERY_CRYSTAL     746
#define ORRERY_VIEWER      770
#define SOUL_CAGE          747
#define MUSIC_BOX          752
#define CALTROPS          756
#define SPELL_BOOK         761
#define SPELL_COMPONENTS   842
#define SMOKE_BOMB         769
#define RUDYOMS_WAND      771
#define BLACK_ROCK         914
#define FLAME_OIL          782
#define HOURGLASS         839
#define ETHERAL_RING       759
#define GENERATORS        981
#define WELL		470
#define ROULETTE	520
#define CLOSED_DOOR	270
#define OPEN_DOOR	376
#define CLOSED_HORIZ_SHUTTERS	290
#define OPEN_HORIZ_SHUTTERS	372
#define CLOSED_VERT_SHUTTERS	291
#define OPEN_VERT_SHUTTERS	322
#define RAKE		620
#define DEAD_BODY	867
#define TREE		453
#define TABLE		1000
#define VERT_BED	1011
#define HORIZ_BED	696
#define STREETLAMP	889
#define HUNG_GARGOYLE	414
#define WATER_TROUGH	719
#define CANDLESTICK	336
#define CHEST		800
#define POTTED_PLANT	999
#define STONE_WALL	192
#define POST		420
#define HORIZ_FENCE	421
#define VERT_FENCE	422
#define FOUNTAIN	893

#if 0 /* ++++++++++More to check: */
545	Curved Heater	3
546	Broken Dish	
547	Magic Sword	7
549	Lightning Whip	
550	Magic Boomerang	
551	Fire Sword	8
552	Magic Axe	8
553	Firedoom Staff	Explosion
554	Burst Arrow	
555	Hawk	
556	Magic Arrows	+4
557	Juggernaut Hammer	
558	Lucky Arrows	
559	Magebane (sword)	
560	Love Arrow	
561	Great Dagger	11
562	The Death Scythe	
563	Blowgun		1
564	Poison Dagger	
565	Starbursts	
566	Glimmer (spell)	
567	Sword of Defense	
568	Tseramed Arrows	8+sleep
569	Leather Armor	1
570	Scale Armor	2
571	Chain Armor	2
572	Wood Shield	2
573	Plate Armor	4
574	Leather Leggings1
575	Chain Leggings	2
576	Plate Leggings	3
577	Pedestal (black gate)	
578	Spiked Shield	2
579	Leather Gloves	1
580	Gauntlets	2
581	Musket Ammo	9
582	Leather Collar	1
583	Bedroll	
584	Kidney belt	1
585	Shoes	
586	Gorget		3
587	Boots	
588	Swamp Boots	
589	Pitchfork	
590	Club		2
591	Main Gauche	2
592	Spear	
593	Throwing Axe	4
594	Dagger		1
595	Torch		3
596	Morning Star	5
597	Bow		8
598	Crossbow	8
599	Sword		6
600	2HD Hammer	9
602	2HD Sword	11
603	Halberd		10
604	Glass Sword	127
605	Bomerang	
606	Magic Bow	12
607	Unknown	
608	Decorative Sword	
609	Kite Shield	
610	Shoreline	
611	Lily Pads	
612	Shoreline	
614	Cleaver	
615	Knife		2
616	Bottle (green)	
617	Abort
618	Scythe	
619	Fern	
#endif

#endif





