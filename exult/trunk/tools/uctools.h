#ifndef UCTOOLS_H
#define UCTOOLS_H

// Opcode flags
// Just a 16bit word
#define IMMED					0
// Will print a part of data string as comment
#define	DATA_STRING				1
// Will add command offset before printing
#define	RELATIVE_JUMP			2
// Will print third byte as decimal after comma
#define CALL					4
// Will print in square brackets
#define VARREF					8
// Will print in square brackets with "flag:" prefix
#define FLGREF					16
// Call of usecode function using extern table
#define EXTCALL					32
// An immediate op. and then a rel. jmp address:  JSF
#define IMMED_AND_RELATIVE_JUMP			64
// Just x bytes
#define PUSH					128
// Just a byte
#define BYTE					256
// variable + relative jump (for the 'sloop')
#define SLOOP		512
#define POP 1024

// Opcode descriptor
typedef struct _opcode_desc
{
	// Mnemonic - NULL if not known yet
	const char* mnemonic;
	// Number of operand bytes
	int nbytes;
	// Type flags
	unsigned int type;
} opcode_desc;

// Opcode table
static opcode_desc opcode_table[] =
{
	{ NULL, 0, 0 },						// 00
	{ NULL, 0, 0 },						// 01
	{ "sloop_iter", 10, SLOOP },		// 02
	{ NULL, 0, 0 },						// 03
	{ "startconv", 2, RELATIVE_JUMP },	// 04
	{ "jne", 2, RELATIVE_JUMP },				// 05
	{ "jmp", 2, RELATIVE_JUMP },				// 06
	{ "cmps", 4, IMMED_AND_RELATIVE_JUMP },			// 07 JSF
	{ NULL, 0, 0 },						// 08
	{ "add", 0, 0 },					// 09
	{ "sub", 0, 0 },					// 0a
	{ "div", 0, 0 },					// 0b
	{ "mul", 0, 0 },					// 0c
	{ "mod", 0, 0 },					// 0d
	{ "and", 0, 0 },					// 0e
	{ "or", 0, 0 },						// 0f
	{ "not", 0, 0 },					// 10
	{ NULL, 0, 0 },						// 11
	{ "pop", 2, VARREF },					// 12
	{ "push\ttrue", 0, 0 },					// 13
	{ "push\tfalse", 0, 0 },				// 14
	{ NULL, 0, 0 },						// 15
	{ "cmpgt", 0, 0 },					// 16
	{ "cmpl", 0, 0 },					// 17
	{ "cmpge", 0, 0 },					// 18
	{ "cmple", 0, 0 },					// 19
	{ "cmpne", 0, 0 },					// 1a
	{ NULL, 0, 0 },						// 1b
	{ "addsi", 2, DATA_STRING },				// 1c
	{ "pushs", 2, DATA_STRING },				// 1d
	{ "arrc", 2, IMMED },					// 1e
	{ "pushi", 2, IMMED },					// 1f
	{ NULL, 0, 0 },						// 20
	{ "push", 2, VARREF },					// 21
	{ "cmpeq", 0, 0 },					// 22
	{ NULL, 0, 0 },						// 23
	{ "call", 2, EXTCALL },					// 24
	{ "ret", 0, 0 },					// 25
	{ "aidx", 2, VARREF },					// 26
	{ NULL, 0, 0 },						// 27
	{ NULL, 0, 0 },						// 28
	{ NULL, 0, 0 },						// 29
	{ NULL, 0, 0 },						// 2a
	{ NULL, 0, 0 },						// 2b
	{ "ret2", 0, 0 },					// 2c
	{ "setr", 0, 0 },					// 2d
	{ "sloop", 11, SLOOP },				// 4e
	{ "addsv", 2, VARREF },				// 2f
	{ "in", 0, 0 },						// 30
	{ "conv_something", 4, IMMED_AND_RELATIVE_JUMP },			// 31
	{ "rts", 0, 0 },					// 32
	{ "say", 0, 0 },					// 33
	{ NULL, 0, 0 },						// 34
	{ NULL, 0, 0 },						// 35
	{ NULL, 0, 0 },						// 36
	{ NULL, 0, 0 },						// 37
	{ "callis", 3, CALL },					// 38
	{ "calli", 3, CALL },					// 39
	{ NULL, 0, 0 },						// 3a
	{ NULL, 0, 0 },						// 3b
	{ NULL, 0, 0 },						// 3c
	{ NULL, 0, 0 },						// 3d
	{ "push\titemref", 0, 0 },				// 3e
	{ "abrt", 0, 0 },					// 3f
	{ "endconv", 0, 0 },					// 40
	{ NULL, 0, 0 },						// 41
	{ "pushf", 2, FLGREF },					// 42
	{ "popf", 2, FLGREF },					// 43
	{ "pushw", 1, BYTE },					// 44
	{ NULL, 0, 0 },						// 45
	{ "setarrayelem", 2, IMMED },			// 46
	{ "calle",2,IMMED },					// 47
	{ "push\teventid", 0, 0 },				// 48
	{ NULL, 0, 0 },						// 49
	{ "arra", 0, 0 },					// 4a
	{ "pop\teventid", 0, 0 },					// 4b
	{ "line",2,IMMED },					// 4c
	{ "func",4,DATA_STRING }			// 4d
};



// Embedded function table

/*
 *	Tables of usecode intrinsics:
 */
#define	USECODE_INTRINSIC_PTR(NAME)	#NAME

const char *bg_intrinsic_table[] =
	{
#include "bgintrinsics.h"
	};
const int bg_intrinsic_size = sizeof(bg_intrinsic_table)/sizeof(char*);
const char *si_intrinsic_table[] = 
	{
#include "siintrinsics.h"
	};
const int si_intrinsic_size = sizeof(si_intrinsic_table)/sizeof(char*);



#if 0
const char* func_table[] = 
{
	"get_random",						// 0
	"execute_usecode_array",				// 1
	"delayed_execute_usecode_array",			// 2
	"show_npc_face",					// 3
	"hide_npc_face",					// 4
	"add_answer",						// 5
	"remove_answer",					// 6
	"push_answers",						// 7
	"pop_answers",						// 8
	"clear_answers",					// 9
	"select_from_menu",					// a
	"select_from_menu2",					// b
	"input_numeric_value",					// c
	"set_item_shape",					// d
	"find_nearest",						// e
	"play_sound_effect",					// f
	"die_roll",						// 10
	"get_item_shape",					// 11
	"get_item_frame",					// 12
	"set_item_frame",					// 13
	"get_item_quality",					// 14
	"set_item_quality",					// 15
	"get_item_quantity",					// 16
	"set_item_quantity",					// 17
	"get_object_position",					// 18
	"get_distance",						// 19
	"find_direction",					// 1a
	"get_npc_object",					// 1b
	"get_schedule_type",					// 1c
	"set_schedule_type",					// 1d
	"add_to_party",						// 1e
	"remove_from_party",					// 1f
	"get_npc_property",					// 20
	"set_npc_property",					// 21
	"get_avatar_ref",					// 22
	"get_party_list",					// 23
	"create_new_object",					// 24
	"set_last_created",					// 25
	"update_last_created",					// 26
	"get_npc_name",						// 27
	"count_objects",					// 28
	"take_from_owner",					// 29
	"get_container_items",					// 2a
	"remove_party_items",					// 2b
	"add_party_items",					// 2c
	NULL,							// 2d
	"play_music",						// 2e
	"npc_nearby",						// 2f
	"find_nearby_avatar",					// 30
	"is_npc",						// 31
	"display_runes",					// 32
	"click_on_item",					// 33
	NULL,							// 34
	"find_nearby",						// 35
	"give_last_created",					// 36
	"is_dead",						// 37
	"game_hour",						// 38
	"game_minute",						// 39
	"get_npc_number",					// 3a
	"part_of_day",						// 3b
	"get_alignment",					// 3c
	"set_alignment",					// 3d
	"move_object",						// 3e
	NULL,							// 3f
	"item_say",						// 40
	"projectile_effect",					// 41
	"get_lift",						// 42
	"set_lift",						// 43
	NULL,							// 44
	NULL,							// 45
	"sit_down",						// 46
	NULL,							// 47
	"display_map",						// 48
	"kill_npc",						// 49
	NULL,							// 4a
	"set_npc_attack_mode",					// 4b
	"set_target_npc_to_attack",				// 4c
	"clone_npc",						// 4d
	NULL,							// 4e
	"show_crystal_ball",					// 4f
	"show_wizard_eye",					// 50
	"resurrect_npc",					// 51
	"add_spell",						// 52
	"sprite_effect",					// 53
	NULL,							// 54
	"book_mode",						// 55
	"stop_time",						// 56
	"cause_light",						// 57
	"get_barge",						// 58
	"earthquake",						// 59
	"is_player_female",					// 5a
	"armageddon",						// 5b
	"halt_scheduled",					// 5c
	"cause_blackout",					// 5d
	"get_array_size",					// 5e
	"mark_stone",						// 5f
	"recall_stone",						// 60
	NULL,							// 61
	"is_pc_inside",						// 62
	"set_orrery_state",					// 63
	NULL,							// 64
	"get_timer",						// 65
	"set_timer",						// 66
	NULL,							// 67
	"mouse_exists",						// 68
	NULL,							// 69
	"flash_mouse",						// 6a
	NULL,							// 6b
	NULL,							// 6c
	NULL,							// 6d
	"get_container",					// 6e
	"remove_item",						// 6f
	NULL,							// 70
	NULL,							// 71
	"get_equipment_list",					// 72
	NULL,							// 73
	NULL,							// 74
	"start_endgame",					// 75
	"fire_cannon",						// 76
	"nap_time",						// 77
	"advance_time",						// 78
	"in_usecode",						// 79
	NULL,							// 7a
	NULL,							// 7b
	NULL,							// 7c
	"path_run_usecode",					// 7d
	"close_gumps",						// 7e
	"item_say",						// 7f
	NULL,							// 80
	"in_gump_mode",						// 81
	NULL,							// 82
	NULL,							// 83
	NULL,							// 84
	"is_not_blocked",					// 85
	NULL,							// 86
	"direction_from",					// 87
	"get_npc_flag",						// 88
	"set_npc_flag",						// 89
	"clear_npc_flag",					// 8a
	"run_usecode",						// 8b
	"fade_palette",						// 8c
	"get_party_list2",					// 8d
	"in_combat",						// 8e
	NULL,							// 8f
	NULL,							// 90
	NULL,							// 91
	NULL,							// 92
	NULL,							// 93
	NULL,							// 94
	NULL,							// 95
	NULL							// 96
};
#endif

#endif
