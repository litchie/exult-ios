//When you are sent to Freedom, the NPC ID of Iolo, Shamino, Dupre and Boydon
//gets set to this value so the game knows they were in your party:
const int BOOTED_FOR_FREEDOM			= 0x001E;
const int CURED_OF_INSANITY				= 0x001D;

//Party members (you know 'em, you love 'em)
enum party_members
{
	PARTY				= -357,	//Used by several intrinsics (e.g. UI_count_objects) that would otherwise take a single NPC
								//Not supported by several other intrinsics that you'd really like it to (e.g. UI_get_cont_items)
	//Permanent NPCs:
	//Britannians:
	AVATAR				= -356,
	DUPRE				= -1,
	SHAMINO				= -2,
	IOLO				= -3,
	GWENNO				= -149,	//Joins after Dupre becomes a pile of ashes
	
	//Serpent Isle:
	BOYDON				= -34,	//Created at Mad Mage Isle, remains until he dies
	
	//Temporary NPCs:
	//Both Joins in Temple of Tolerance, both die soon after:
	MORTEGRO			= -26,
	SETHYS				= -152,
	
	PETRA				= -28,	//Joins for Temple of Discipline, remains until kicked out
	
	SELINA				= -44,	//Joins for the Mint subquest, leaves after
	
	WILFRED				= -45,	//Joins after banes, bolts if hurt too badly
	
	STEFANO				= -168	//Joins in freedom (leaves after) and again after banes until you kill the Death Knight

};

//Moonshade residents
enum moonshade_npcs
{
	ANDRIO				= -4,
	SERV_01				= -5,
	SERV_02				= -6,
	SERV_03				= -7,
	SERV_04				= -8,
	SERV_05				= -9,
	SERV_06				= -10,
	SERV_07				= -11,
	SERV_08				= -12,
	BUCIA				= -13,
	COLUMNA				= -14,
	DUCIO				= -15,
	EDRIN				= -16,
	FEDABIBLIO			= -17,
	FILBERCIO			= -18,
	FRELI				= -19,
	FRIGIDAZZI			= -20,
	GOBLIN_MESSENGER	= -21,
	GUSTACIO			= -22,
	JULIA				= -23,
	MELINO				= -24,
	AUTO_MESSENGER		= -25,
	//MORTEGRO			= -26,	//Defined above
	MOSH				= -27,
	//PETRA				= -28,	//Defined above
	POTHOS				= -29,
	ROCCO				= -30,
	ROTOLUNCIA			= -31,
	TOPO				= -32,
	TORRISSIO			= -33,
	SERV09				= -191,
	SERV10				= -192,
	SERV11				= -193,
	SERV12				= -194,
	MUSIC01				= -198,
	MUSIC02				= -199,
	MUSIC03				= -200,
	SERV_SEW			= -206,
	AUTOGUARD			= -216
};

//Mad Mage Isle residents
enum madmage_npcs
{
	//BOYDON			= -34,	//Defined above
	ERSTAM				= -35,
	VASEL				= -36
};

//Sleeping Bull residents
enum sleepingbull_npcs
{
	ALE					= -37,
	ARGUS				= -38,
	BYRIN				= -39,
	HAWK				= -40,
	DEVRA				= -41,
	FLINDO				= -42,
	KANE				= -43,
	//SELINA			= -44,	//Defined as one of Batlin's goons, below
	//WILFRED			= -45,	//Defined above
	ENSORCIO			= -81
};

//Fawn residents, including the Fellowship members
enum fawn_npcs
{
	ALYSSAND			= -46,
	DELIN				= -47,
	DELPHYNIA			= -48,
	GARTH				= -49,
	JOTH				= -50,
	VOLDIN				= -51,
	JENDON				= -52,
	JORVIN				= -53,
	KYLISTA				= -54,
	YELINDA				= -55,
	LEON				= -56,
	KALEN				= -57,
	OLON				= -58,
	RUGGS				= -59,
	SCOTS				= -60,
	ZULITH				= -61,
	BUSTER				= -260	//Dog
};

//Monitor residents
enum monitor_npcs
{
	ANDRAL				= -62,
	CALADIN				= -63,
	CANTRA				= -64,
	CELLIA				= -65,
	G_SIMON				= -66,	//Simon the Goblin
	HARNNA				= -67,
	KRAYG				= -68,
	MARSTEN				= -69,
	LUCILLA				= -70,
	LUTHER				= -71,
	LYDIA				= -72,
	RENFRY				= -73,
	SHAZZANA			= -74,
	SCHMED				= -75,	//Technically from Monitor, although very far from home...
	SIMON				= -76,	//Simon in disguise
	SPEKTOR				= -77,
	STANDARR			= -78,
	TEMPLAR				= -79,
	FLICKEN				= -80,	//Monitor's west gate guard
	BRENDANN			= -150,
	SMUDGE				= -261	//Cat
};

//Automatons in the temple of discipline
enum temple_of_discipline
{
	LORD_OF_DISCIPLINE	= -82,	//Failing automaton
	ACID_AUTOMATON		= -83,	//Another failing automaton
	GUARD1				= -84,
	GUARD3				= -86,
	GUARD5				= -88,
	GUARD6				= -89,
	GUARD7				= -90,
	GUARD8				= -91
};

//Furnace residents
enum furnace
{
	AUTO1				= -85,
	AUTO2				= -87,
	AUTO3				= -92,
	AUTO4				= -93,
	ZHELKAS				= -170,
	THIEF1				= -176,
	THIEF2				= -177,
	THIEF3				= -183,
	KEY_GUY				= -259	//Troll, has key to Serpent Staff
};

//Automatons from temple of logic
enum temple_of_logic
{
	MURDER1				= -94,
	MURDER2				= -95,
	MURDER3				= -96,
	MURDER4				= -97,
	MURDER5				= -98,
	MURDER6				= -99,
	MURDER7				= -184
};

//Residents from temple of ethicality
enum temple_of_ethicality
{
	ETH_AUTOMATON			= -100,	//The automaton which administers the test of ethicality
	MAN_IN_FIRE				= -151
};


//Goblin camp residents
enum goblin_camp
{
	GOBL01				= -101,
	GOBL02				= -102,
	GOBL03				= -103,
	GOBL04				= -104,
	GOBL05				= -105,
	GOBL06				= -106,
	GOBL07				= -107,
	GOBL08				= -108,
	GOBL09				= -109,
	GOBL10				= -110,
	GOBL11				= -111,
	GOBL12				= -112,
	GOBL13				= -113,
	GOBL14				= -114,
	GOBL15				= -115,
	GOBL16				= -116,
	GOBL17				= -117,
	GOBL18				= -118,
	GOBLIN_KING			= -265,
	CHUCK				= -274,
	JOHNSON				= -285
};

//Bull Tower residents, the five goblins you must kill to free the tower
enum bull_tower
{
	GOBL19				= -119,
	GOBL21				= -120,
	GOBL22				= -121,
	GOBL23				= -122,
	GOBL24				= -123
};

//Dream world denizens
enum dream_world
{
	D_STEFANO			= -124,
	RABINDRINATH		= -181,
	SIRANUSH			= -182,
	D_CANTRA			= -201,
	D_BYRIN				= -218,
	D_ENSORCIO			= -219,
	D_FILBERTIO			= -220,
	SMITHZHORSE			= -277,
	LORD_BRITISH		= -280
};

//Spinebreaker residents
enum spinebreaker
{
	BUTLER				= -125,	//The hierophant's butler
	GUARD_11			= -126,	//Greets you with Batlin's voice
	//BRUNT				= -127,	//Defined below
	//DEADEYE			= -128,	//Defined below
	PASSWORD			= -129,	//Asks for the password at entrance
	METAL_MAN			= -202	//Asks about The Structure of Order
};

//Skullcrusher residents
enum skullcrusher
{
	GWANI_01			= -140,
	GWANI_02			= -141,
	GWANI_03			= -142,
	GUARD18				= -186,
	GUARD19				= -187,
	GUARD20				= -188,
	GUARD21				= -189,
	GUARD17				= -190,
	VASCULIO			= -294
};

//Gwani Village residents
enum gwanni_npcs
{
	BAYANDA				= -143,
	BWUNDIAI			= -144,
	MWAERNO				= -145,
	MYAURI				= -146,
	NEYOBI				= -147,
	YENANI				= -148,
	GILWOYAI			= -153,
	KAPYUNDI			= -154
};

//Residents from temple of tolerance
enum temple_of_tolerance
{
	//SETHYS			= -152,	//Defined above
	YEARL				= -267,	//Snow leopard
	BRENDA				= -268,	//Naga
	MEELOSE				= -273,	//Wildman
	EMMIT				= -279	//Ratman
};

//Great Northern Forest residents
enum northernforest_npcs
{
	BERYL				= -155,
	DRAYGAN				= -156,
	HURD				= -157,
	IVOR				= -158,
	MORGHRIM			= -159,
	HOUND_OS_DOSKAR		= -269,
	HAZARD				= -275,
	SONAR				= -276	//Wolf
};

//Freedom residents
enum mountains_of_freedom
{
	JAIL_01				= -160,
	JAIL_02				= -161,
	JAIL_03				= -162,
	LORTHONDO			= -163,
	PRISON1				= -164,
	PRISON2				= -165,
	PRISON3				= -166,
	PRISON4				= -167,
	TELDRONO			= -169,
	PIRATE_GUY			= -196,
	SABRINA				= -217,
	GARG_PRISONER		= -266,
	WATSON				= -282,
	DEATHMARE			= -283
};

//Test of Purity residents
enum test_of_purity
{
	SEX_01				= -171,
	SEX_02				= -172,
	SEX_03				= -173,
	SEX_04				= -174,
	SEX_05				= -175,
	CLONE_IOLO			= -178,	//Also appears in Dream World
	CLONE_SHAMINO		= -179,
	CLONE_DUPRE			= -180
};

//Shamino's castle
enum shamino_castle
{
	HENCH1				= -205,
	HENCH2				= -203,	//Starts at House of the Dead
	HENCH3				= -204	//Starts at House of the Dead
};

//Monk Isle residents
enum monkisle_npcs
{
	KARNAX				= -207,
	SILENT2				= -208,
	MIGGIM				= -209,
	SILENT3				= -210,
	THOXA				= -211,
	BRACCUS				= -212,
	DRAXTA				= -213,
	SILENT1				= -214,
	XENKA				= -215
};

//Residents of Castle of the White Dragon
enum white_dragon_castle
{
	//The quotes are from http://www.it-he.org
	ANTI_DUPRE			= -263,	//Haha! I am Dupre, bane of drunkenness!
	ANTI_IOLO			= -262,	//I am Iolo! Bane of shooting-the-Avatar-through-the-heart-with-the-triple-crossbow!
	ANTI_SHAM			= -264	//Kneel before Shamino, bane of rapidly diminishing consequence...
};

//Batlin's goons
enum batlingoons_npcs
{
	//Batlin is actually a monster in this game, not a real person... err, NPC.
	//SELINA			= -44,	//Defined above
	BRUNT				= -127,
	DEADEYE				= -128,
	PALOS				= -272
};

//All other NPCs
enum misc_npcs
{
	GUARD12				= -130,	//Automaton, starts at House of the Dead
	GUARD13				= -131,	//Automaton, starts at House of the Dead
	GUARD14				= -132,	//Automaton, starts at House of the Dead
	GUARD15				= -133,	//Automaton, starts at House of the Dead
	GUARD16				= -134,	//Automaton, starts at House of the Dead
	ZOMBIE01			= -135,	//Gwani, starts at House of the Dead
	ZOMBIE02			= -136,	//Gwani, starts at House of the Dead
	ZOMBIE03			= -137,	//Gwani, starts at House of the Dead
	ZOMBIE04			= -138,	//Gwani, starts at House of the Dead
	ZOMBIE05			= -139,	//Gwani, starts at House of the Dead
	DED_AUTOMATON		= -185,	//Automaton, in the middle of ocean
	PERRY_STOKES		= -195,	//The software pirate
	IAUTO				= -197,	//Automaton, starts at House of the Dead
	DEATH_KNIGHT		= -222,	//Starts at the House of the Dead
	LOAF1				= -221,	//Starts at the House of the Dead
	LOAF2				= -223,	//Starts at the House of the Dead
	LOAF3				= -224,	//Starts at the House of the Dead
	LOAF4				= -225,	//Starts at the House of the Dead
	LOAF5				= -226,	//Starts at the House of the Dead
	LOAF6				= -227,	//Starts at the House of the Dead
	AUTO_2				= -228,	//Automaton, starts at the House of the Dead
	AUTO_3				= -229,	//Automaton, starts at the House of the Dead
	AUTO_4				= -230,	//Automaton, starts at the House of the Dead
	LARRY				= -231,	//Automaton, starts at the House of the Dead
	STAN				= -232,	//Automaton, starts at the House of the Dead
	SUPER_GOBLIN		= -256,	//At the tree leading to Goblin Camp
	BILLY_CAIN			= -257,	//Goblin, SW of knight's test
	STEVE_POWERS		= -258,	//Goblin, SE of Sleeping Bull Inn, along the north coast
	HENCH_MAN			= -270,	//Starts at the House of the Dead
	HENCHMAN			= -271,	//Starts at the House of the Dead
	ELSI				= -278,	//Cow, north of Mint
	CRUSTY				= -281,	//Clown, SW of Captain Stokes tower
	PIBB				= -284,	//Starts at the House of the Dead
	CINDY				= -287,	//Naked girl S of turtle-summoning bell
	DBEYER				= -293,	//Gargoyle, starts at the House of the Dead
	FITCH				= -295	//Trapper, the one dying at the edge of the Ice Plains
};
