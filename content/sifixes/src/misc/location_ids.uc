enum location_ids
{
	THE_WILDERNESS				= 0xFFFF,
	WHITE_DRAGON_CASTLE			= 0x0000,
	CLAW						= 0x0001,	//Defined in "In Wis" usecode, but not in the function below
	MONK_ISLE					= 0x0002,
	FAWN						= 0x0003,
	FURNACE						= 0x0004,
	GLACIER_MOUNTAINS			= 0x0005,
	GRAND_SHRINE				= 0x0006,	//Defined in "In Wis" usecode, but not in the function below
	GREAT_NORTHERN_FOREST		= 0x0007,
	ICE_PLAINS					= 0x0008,
	SLEEPING_BULL_INN			= 0x0009,
	ISLE_OF_CRYPTS				= 0x000A,
	MAD_MAGE_ISLAND				= 0x000B,
	KNIGHTS_TEST				= 0x000C,
	MONITOR						= 0x000D,
	MOONSHADE					= 0x000E,
	FREEDOM_MOUNTAINS			= 0x000F,
	SHAMINOS_CASTLE				= 0x0010,
	DISCIPLINE					= 0x0011,
	EMOTION						= 0x0012,
	ENTHUSIASM					= 0x0013,
	ETHICALITY					= 0x0014,
	LOGIC						= 0x0015,
	TOLERANCE					= 0x0016,
	SKULLCRUSHER_MOUNTAINS		= 0x0017,
	SPINEBREAKER_MOUNTAINS		= 0x0018,
	SUNRISE_ISLE				= 0x0019,
	GORLAB_SWAMP				= 0x001A,
	STARTING_AREA				= 0x001B,
	WESTERN_FOREST				= 0x001C,
	GWANNI_VILLAGE				= 0x001D,
	PROGRAMMERS_ISLAND			= 0x001E,
	DREAM_WORLD					= 0x001F,
	SILVER_SEED					= 0x0020,
	THE_MAZE					= 0x0021
};

var getLocationID 0x993 (var pos)
{
	var pos_x = pos[X];
	var pos_y = pos[Y];
	var location_id = THE_WILDERNESS;

	if (((pos_x > 0x06AE) && (pos_x < 0x073F) && (pos_y > 0x053C) && (pos_y < 0x05AF)) || ((pos_x > 0x098B) && (pos_x < 0x09E3) && (pos_y > 0x001C) && (pos_y < 0x007E)))
		location_id = WHITE_DRAGON_CASTLE;

	else if (((pos_x > 0x08C4) && (pos_x < 0x09FE) && (pos_y > 0x03A1) && (pos_y < 0x04FE)) || ((pos_x > 0x0A20) && (pos_x < 0x0AFF) && (pos_y > 0x0400) && (pos_y < 0x04DF)))
		location_id = MONK_ISLE;

	else if (((pos_x > 0x0AFA) && (pos_x < 0x0BFF) && (pos_y > 0x0400) && (pos_y < 0x0600)) || ((pos_x > 0x0000) && (pos_x < 0x00FF) && (pos_y > 0x0401) && (pos_y < 0x05FF)))
		location_id = DREAM_WORLD;

	else if (((pos_x > 0x035A) && (pos_x < 0x04A3) && (pos_y > 0x05FD) && (pos_y < 0x07D7)) || ((pos_x > 0x0AC0) && (pos_y > 0x00C0) && (pos_x < 0x0ACF) && (pos_y < 0x00DF)))
		location_id = FAWN;

	else if (((pos_x > 0x0480) && (pos_x < 0x057F) && (pos_y > 0x0A00) && (pos_y < 0x0AAF)) || ((pos_x > 0x0580) && (pos_x < 0x07CF) && (pos_y > 0x0970) && (pos_y < 0x0ABF)) || ((pos_x > 0x0790) && (pos_x < 0x07EC) && (pos_y > 0x0AB0) && (pos_y < 0x0AFC)) || ((pos_x > 0x0B20) && (pos_x < 0x0BDF) && (pos_y > 0x0710) && (pos_y < 0x07BC)) || ((pos_x > 0x000D) && (pos_x < 0x00AB) && (pos_y > 0x061F) && (pos_y < 0x067B)) || ((pos_x > 0x0A0D) && (pos_y > 0x005E) && (pos_x < 0x0A3F) && (pos_y < 0x007F)) || ((pos_x > 0x0A33) && (pos_y > 0x009F) && (pos_x < 0x0A5F) && (pos_y < 0x00AF)))
		location_id = FURNACE;

	else if ((pos_x > 0x0A8F) && (pos_x < 0x0AE0) && (pos_y > 0x004F) && (pos_y < 0x00A0))
		location_id = LOGIC;

	else if (((pos_x > 0x05C0) && (pos_x < 0x06F8) && (pos_y > 0x01DC) && (pos_y < 0x0236)) || ((pos_x > 0x0550) && (pos_x < 0x0590) && (pos_y > 0x01D0) && (pos_y < 0x0210)))
		location_id = TOLERANCE;

	else if ((pos_x > 0x0530) && (pos_x < 0x0640) && (pos_y > 0x0211) && (pos_y < 0x0370))
		location_id = GLACIER_MOUNTAINS;

	else if (((pos_x > 0x0450) && (pos_x < 0x069F) && (pos_y > 0x04C0) && (pos_y < 0x055F)) || ((pos_x > 0x0230) && (pos_x < 0x0320) && (pos_y > 0x0640) && (pos_y < 0x06B3)) || ((pos_x > 0x05FA) && (pos_x < 0x06F0) && (pos_y > 0x03FC) && (pos_y < 0x04BF)))
		location_id = GREAT_NORTHERN_FOREST;

	else if (((pos_x > 0x0490) && (pos_x < 0x0530) && (pos_y > 0x0860) && (pos_y < 0x08F0)) || ((pos_x > 0x0AF0) && (pos_y > 0x00C0) && (pos_x < 0x0AFF) && (pos_y < 0x00DF)))
		location_id = SLEEPING_BULL_INN;

	else if ((pos_x > 0x0070) && (pos_x < 0x0230) && (pos_y > 0x0320) && (pos_y < 0x0410))
		location_id = ISLE_OF_CRYPTS;

	else if (((pos_x > 0x0780) && (pos_x < 0x0850) && (pos_y > 0x04F0) && (pos_y < 0x0571)) || ((pos_x > 0x07D0) && (pos_x < 0x0880) && (pos_y > 0x0442) && (pos_y < 0x04F6)))
		location_id = MAD_MAGE_ISLAND;

	else if ((pos_x > 0x0310) && (pos_x < 0x0400) && (pos_y > 0x0800) && (pos_y < 0x08E8))
		location_id = KNIGHTS_TEST;

	else if (((pos_x > 0x02A0) && (pos_x < 0x0470) && (pos_y > 0x0980) && (pos_y < 0x0AF0)) || ((pos_x > 0x0B1E) && (pos_y > 0x00AE) && (pos_x < 0x0B30) && (pos_y < 0x00D1)))
		location_id = MONITOR;

	else if ((pos_x > 0x0834) && (pos_x < 0x09BE) && (pos_y > 0x06BB) && (pos_y < 0x084E))
		location_id = MOONSHADE;

	else if (((pos_x > 0x07E0) && (pos_x < 0x08D0) && (pos_y > 0x0580) && (pos_y < 0x0690)) || ((pos_x > 0x0870) && (pos_x < 0x0910) && (pos_y > 0x0520) && (pos_y < 0x05E0)) || ((pos_x > 0x08C0) && (pos_x < 0x0A20) && (pos_y > 0x0520) && (pos_y < 0x0591)) || ((pos_x > 0x0960) && (pos_x < 0x0A60) && (pos_y > 0x0590) && (pos_y < 0x0650)) || ((pos_x > 0x09F0) && (pos_x < 0x0A60) && (pos_y > 0x0640) && (pos_y < 0x06D0)) || ((pos_x > 0x09B0) && (pos_x < 0x0A10) && (pos_y > 0x06A0) && (pos_y < 0x0710)))
		location_id = FREEDOM_MOUNTAINS;

	else if (((pos_x > 0x0708) && (pos_x < 0x07BA) && (pos_y > 0x03B4) && (pos_y < 0x0454)) || ((pos_x > 0x0B93) && (pos_y > 0x0043) && (pos_x < 0x0BEC) && (pos_y < 0x00EC)))
		location_id = SHAMINOS_CASTLE;

	else if (((pos_x > 0x06E0) && (pos_x < 0x072F) && (pos_y > 0x0250) && (pos_y < 0x02AF)) || ((pos_x > 0x0B00) && (pos_x < 0x0B8F) && (pos_y > 0x0050) && (pos_y < 0x007F)))
		location_id = DISCIPLINE;

	else if ((pos_x > 0x0580) && (pos_x < 0x05BF) && (pos_y > 0x0390) && (pos_y < 0x03EF))
		location_id = EMOTION;

	else if ((pos_x > 0x08C0) && (pos_x < 0x093F) && (pos_y > 0x0120) && (pos_y < 0x014F))
		location_id = ENTHUSIASM;

	else if (((pos_x > 0x08B0) && (pos_x < 0x08FF) && (pos_y > 0x0310) && (pos_y < 0x035F)) || ((pos_x > 0x09A0) && (pos_x < 0x0AFF) && (pos_y > 0x0300) && (pos_y < 0x038F)))
		location_id = ETHICALITY;

	else if (((pos_x > 0x03A0) && (pos_x < 0x044F) && (pos_y > 0x03D0) && (pos_y < 0x052F)) || ((pos_x > 0x0450) && (pos_x < 0x056F) && (pos_y > 0x03F0) && (pos_y < 0x049F)) || ((pos_x > 0x04A0) && (pos_x < 0x050F) && (pos_y > 0x0270) && (pos_y < 0x03FF)) || ((pos_x > 0x0450) && (pos_x < 0x04CF) && (pos_y > 0x0260) && (pos_y < 0x02FF)) || ((pos_x > 0x0B10) && (pos_x < 0x00F0) && (pos_y > 0x0800) && (pos_y < 0x08F0)) || ((pos_x > 0x0000) && (pos_x < 0x00C0) && (pos_y > 0x0700) && (pos_y < 0x0800)))
		location_id = SKULLCRUSHER_MOUNTAINS;

	else if (((pos_x > 0x0730) && (pos_x < 0x095E) && (pos_y > 0x0208) && (pos_y < 0x02F8)) || ((pos_x > 0x087E) && (pos_x < 0x095E) && (pos_y > 0x0168) && (pos_y < 0x0208)))
		location_id = SPINEBREAKER_MOUNTAINS;

	else if ((pos_x > 0x0500) && (pos_x < 0x07EE) && (pos_y > 0x0000) && (pos_y < 0x0190))
		location_id = SUNRISE_ISLE;

	else if (((pos_x > 0x0540) && (pos_x < 0x05F0) && (pos_y > 0x0580) && (pos_y < 0x0670)) || ((pos_x > 0x05F0) && (pos_x < 0x06EA) && (pos_y > 0x055A) && (pos_y < 0x0640)))
		location_id = GORLAB_SWAMP;

	else if ((pos_x > 0x0210) && (pos_x < 0x02D0) && (pos_y > 0x0950) && (pos_y < 0x0AB0))
		location_id = STARTING_AREA;

	else if ((pos_x > 0x0258) && (pos_x < 0x030C) && (pos_y > 0x047E) && (pos_y < 0x05C8))
		location_id = WESTERN_FOREST;

	else if ((pos_x > 0x037A) && (pos_x < 0x0488) && (pos_y > 0x033E) && (pos_y < 0x037A))
		location_id = GWANNI_VILLAGE;

	//This is in parts of the goblin caves; there was probably something else there at another stage...
	//else if ((pos_x > 0x012F) && (pos_x < 0x01DF) && (pos_y > 0x072F) && (pos_y < 0x07C0))
	//	location_id = PROGRAMMERS_ISLAND;

	else if (((pos_x > 0x0000) && (pos_y > 0x0AFF) && (pos_x < 0x0900) && (pos_y < 0x0C00)) || ((pos_x > 0x08FF) && (pos_y > 0x09FF) && (pos_x < 0x0C00) && (pos_y < 0x0C00)) || ((pos_x > 0x0B6F) && (pos_y > 0x09BF) && (pos_x < 0x0C00) && (pos_y < 0x0A00)))
		location_id = SILVER_SEED;

	//Doesn't ever seem to get used in the original (and in fact, it seems to
	//mess up dying in the Maze -- you can end up reviving near Fawn...).
	//I have commented it out, but given the way I placed it, it will never
	//get used anyway...
	//else if ((pos_x > 0x08FF) && (pos_y > 0x09FF) && (pos_x < 0x0A00) && (pos_y < 0x0B00))
	//	location_id = THE_MAZE;

	else if (((pos_x > 0x0A13) && (pos_y > 0x0014) && (pos_x < 0x0A3F) && (pos_y < 0x003D)) || ((pos_x > 0x09D0) && (pos_y > 0x00A0) && (pos_x < 0x09EE) && (pos_y < 0x00AE)) || ((pos_x > 0x0250) && (pos_y > 0x033F) && (pos_x < 0x0330) && (pos_y < 0x0481)))
		location_id = THE_WILDERNESS;

	else if (pos_y < 0x0400)
		location_id = ICE_PLAINS;

	return location_id;
}
