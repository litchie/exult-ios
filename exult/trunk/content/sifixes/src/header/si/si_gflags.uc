//This file lists SI flags. The goal is to list them all; one day, they will be...

//Temporary flags:
const int TEMP_FLAG_1					= 0x7;		//Temporary flag, used for a great many things
const int TEMP_FLAG_2					= 0x8;		//Temporary flag, used for a great many things
const int TEMP_FLAG_3					= 0x9;		//Temporary flag, used for a great many things
const int TEMP_FLAG_4					= 0xA;		//Temporary flag, used for a great many things


const int STARTING_SPEECH				= 0x3;
const int BANES_RELEASED				= 0x4;
const int EQUIPMENT_EXCHANGED			= 0x6;
const int GWENNO_HAS_BELONGINGS			= 0x10;
const int DUPRE_HAS_BELONGINGS			= 0x17;
const int SHAMINO_HAS_BELONGINGS		= 0x18;
const int IOLO_HAS_BELONGINGS			= 0x19;
const int GOBLIN_SIMON_DEAD				= 0x44;
const int CURED_CANTRA					= 0x47;		// Flag when Cantra is cured.
const int AVATAR_IS_KNIGHT				= 0x48;
const int SLAIN_WOLF					= 0x4A;
const int KNOWS_KRAYG_WALKS_ON_WOODS	= 0x4E;
const int AVATAR_GOT_SHORT_STICK		= 0x60;
const int DUPRE_IS_TOAST				= 0x61;
const int IOLO_MADE_EQUIPMENT_LIST		= 0x78;
const int KNOWS_CANTRA_IS_MISSING		= 0x79;
const int BEGAN_KNIGHTS_TEST			= 0x82;
const int MONITOR_TRAINING				= 0x83;
const int HAS_DUPRE_SHIELD				= 0xB2;
const int DUPRE_MADE_EQUIPMENT_LIST		= 0xB7;
const int MONITOR_BANQUET_STARTED		= 0xBF;

//The quotes are from http://www.it-he.org
const int WANTONESS_BANE_DEAD			= 0xD3;		//Haha! I am Dupre, bane of drunkenness!
const int INSANITY_BANE_DEAD			= 0xD5;		//I am Iolo! Bane of shooting-the-Avatar-through-the-heart-with-the-triple-crossbow!
const int ANARCHY_BANE_DEAD				= 0xD4;		//Kneel before Shamino, bane of rapidly diminishing consequence...

const int KNOWS_OF_SOUL_GEMS			= 0xFF;
const int SHAMINO_RESURRECTED_BY_MONKS	= 0x12B;
const int SEEKING_SOUT_TRAP_SECRET		= 0x12D;
const int AFTER_FREEDOM_NEWS			= 0x135;
const int SHAMINO_TELEPORTED_BY_MONKS	= 0x13A;
const int HAVE_CHILL_SPELL				= 0x14B;
const int CLEARED_FAWN_TOWER			= 0x14E;
const int TALKED_TO_CHAOS_HIEROPHANT	= 0x22B;
const int BEATRIX_PROTECTION			= 0x25B;
const int BEATRIX_FORGAVE_SHAMINO		= 0x25C;
const int FREED_GWENNOS_BODY			= 0x25F;
const int TALKED_TO_GWANI_ABOUT_GWENNO	= 0x262;
const int BAYANDA_GAVE_BUCKET			= 0x264;
const int GWENNO_IS_DEAD				= 0x26A;
const int IOLO_GWENNO_REUNITED			= 0x275;	//Iolo and Gwenno are alive, sane and talked to each other
const int TALKED_TO_GREAT_HIEROPHANT	= 0x279;
const int SHAMINO_MADE_EQUIPMENT_LIST	= 0x2A2;
const int SERPENT_GWANI_HORN_SPEECH		= 0x2DC;
const int SERPENT_GWENNO_BANE_SPEECH	= 0x2DD;	//GES talked about banes after Gwenno got resurrected

//Songs (Byrin and Iolo):
const int HEARD_BEATRIX_SONG			= 0x1D3;
const int HEARD_MOUNTAIN_SONG			= 0x1D4;
const int HEARD_GWANI_SONG				= 0x1D5;
const int HEARD_FOREST_MASTER_SONG		= 0x1D6;
const int HEARD_DREAM_SONG				= 0x1D7;
const int HEARD_WHITE_DRAGON_SONG		= 0x1D8;

//Flags for the teleport storm items:
const int STORM_PINECONE				= 0x27A;
const int STORM_BLUE_EGG				= 0x283;
const int STORM_RUDDY_ROCK				= 0x28B;
const int STORM_LAB_APPARATUS			= 0x27D;
const int STORM_PUMICE					= 0x27E;
const int STORM_GOBLIN_BRUSH			= 0x284;
const int STORM_WEDDING_RING			= 0x27F;
const int STORM_STOCKINGS				= 0x27B;
const int STORM_BEAR_SKULL				= 0x288;
const int STORM_MONITOR_SHIELD			= 0x28A;
const int STORM_ICEWINE					= 0x285;
const int STORM_URN						= 0x27C;
const int STORM_SLIPPERS				= 0x281;
const int STORM_FILARI					= 0x287;
const int STORM_SEVERED_HAND			= 0x289;
const int STORM_FUR_CAP					= 0x280;
const int STORM_BREAST_PLATE			= 0x282;

//Flags identifying the teleport storm items:
const int KNOWS_PINECONE_OWNER			= 0x28C;
const int KNOWS_BLUE_EGG_OWNER			= 0x295;
const int KNOWS_STONEHEART_ORIGIN		= 0x29D;
const int HAS_CLUE_LAB_APPARATUS		= 0x110;
const int KNOWS_LAB_APPARATUS_OWNER		= 0x28F;
const int KNOWS_PUMICE_ORIGIN			= 0x290;
const int KNOWS_GOBLIN_BRUSH_ORIGIN		= 0x296;
const int HAS_CLUE_RING					= 0x14C;
const int KNOWS_RING_OWNER				= 0x291;
const int KNOWS_MOONSILK_OWNER			= 0x28D;
const int KNOWS_BEAR_SKULL_ORIGIN		= 0x29A;
const int HAS_CLUE_MONITOR_SIHELD		= 0xAC;
const int KNOWS_MONITOR_SHIELD_ORIGIN	= 0x29C;
const int KNOWS_ICEWINE_ORIGIN			= 0x297;
const int KNOWS_URN_ORIGIN				= 0x28E;
const int KNOWS_SLIPPERS_OWNER			= 0x293;
const int KNOWS_FILARI_OWNER			= 0x299;
const int KNOWS_SEVERED_HAND_OWNER		= 0x29B;
const int HAS_CLUE_FURCAP				= 0x2A0;
const int KNOWS_FURCAP_OWNER			= 0x292;
const int KNOWS_BREAST_PLATE_OWNER		= 0x294;

//New flag to fix bug with fur cap:
const int GAVE_FURCAP_BACK				= 0x350;
