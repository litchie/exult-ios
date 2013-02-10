#ifndef UCTOOLS_H
#define UCTOOLS_H

// Opcode flags
enum Opcode_flags {
    // Just a 16bit word
    op_immed = 0,
    // Will print a part of data string as comment
    op_data_string = 1,
    // Will add command offset before printing
    op_relative_jump = 2,
    // Will print third byte as decimal after comma
    op_call = 3,
    // Will print in square brackets
    op_varref = 4,
    // Will print in square brackets with "flag:" prefix
    op_flgref = 5,
    // Call of usecode function using extern table
    op_extcall = 6,
    // An immediate op. and then a rel. jmp address:  JSF
    op_immed_and_relative_jump = 7,
    // Just x bytes
    op_push = 8,
    // Just a byte
    op_byte = 9,
    // variable + relative jump (for the 'sloop')
    op_sloop = 10,
    op_pop = 11,
    op_immed32 = 12,
    op_data_string32 = 13,
    op_relative_jump32 = 14,
    op_immedreljump32 = 15,
    op_sloop32 = 16,
    op_staticref = 17,
    op_classvarref = 18,
    op_immed_pair = 19,
    op_argnum_reljump = 20,
    op_argnum_reljump32 = 21,
    op_argnum = 22,
    op_funid = 23,
    op_funcall = 24,
    op_clsfun = 25,
    op_clsfun_vtbl = 26,
    op_static_sloop = 27,
    op_clsid = 28,
    op_unconditional_jump = 29,
    op_uncond_jump32 = 30,
	op_funid32 = 31
};

// Opcode descriptor
typedef struct _opcode_desc {
	// Mnemonic - NULL if not known yet
	const char *mnemonic;
	// Number of operand bytes
	int nbytes;
	// Type flags
	unsigned int type;
	// Number of elements poped from stack
	int num_pop;
	// Number of elements pushed into stack
	int num_push;
} opcode_desc;

// Opcode table
static opcode_desc opcode_table[] = {
	{ NULL, 0, 0, 0, 0 },                       // 00
	{ NULL, 0, 0, 0, 0 },                       // 01
	{ "loop", 10, op_sloop, 0, 0 },     // 02
	{ NULL, 0, 0, 0, 0 },                       // 03
	{ "startconv", 2, op_relative_jump, 0, 0 }, // 04
	{ "jne", 2, op_relative_jump, 1, 0 },               // 05
	{ "jmp", 2, op_unconditional_jump, 0, 0 },              // 06
	{ "cmps", 4, op_argnum_reljump, 0, 0 },         // 07 JSF
	{ NULL, 0, 0, 0, 0 },                       // 08
	{ "add", 0, 0, 2, 1 },                  // 09
	{ "sub", 0, 0, 2, 1 },                  // 0a
	{ "div", 0, 0, 2, 1 },                  // 0b
	{ "mul", 0, 0, 2, 1 },                  // 0c
	{ "mod", 0, 0, 2, 1 },                  // 0d
	{ "and", 0, 0, 2, 1 },                  // 0e
	{ "or", 0, 0, 2, 1 },                       // 0f
	{ "not", 0, 0, 1, 1 },                  // 10
	{ NULL, 0, 0, 0, 0 },                       // 11
	{ "pop", 2, op_varref, 1, 0 },                  // 12
	{ "push\ttrue", 0, 0, 0, 1 },                   // 13
	{ "push\tfalse", 0, 0, 0, 1 },              // 14
	{ NULL, 0, 0, 0, 0 },                       // 15
	{ "cmpgt", 0, 0, 2, 1 },                    // 16
	{ "cmplt", 0, 0, 2, 1 },                 // 17
	{ "cmpge", 0, 0, 2, 1 },                    // 18
	{ "cmple", 0, 0, 2, 1 },                    // 19
	{ "cmpne", 0, 0, 2, 1 },                    // 1a
	{ NULL, 0, 0, 0, 0 },                       // 1b
	{ "addsi", 2, op_data_string, 0, 0 },               // 1c
	{ "pushs", 2, op_data_string, 0, 1 },               // 1d
	{ "arrc", 2, op_argnum, 0, 1 },                 // 1e
	{ "pushi", 2, op_immed, 0, 1 },                 // 1f
	{ NULL, 0, 0, 0, 0 },                       // 20
	{ "push", 2, op_varref, 0, 1 },                 // 21
	{ "cmpeq", 0, 0, 2, 1 },                    // 22
	{ NULL, 0, 0, 0, 0 },                       // 23
	{ "call", 2, op_extcall, 0, 0 },                    // 24
	{ "ret", 0, 0, 0, 0 },                  // 25
	{ "aidx", 2, op_varref, 1, 1 },                 // 26
	{ NULL, 0, 0, 0, 0 },                       // 27
	{ NULL, 0, 0, 0, 0 },                       // 28
	{ NULL, 0, 0, 0, 0 },                       // 29
	{ NULL, 0, 0, 0, 0 },                       // 2a
	{ NULL, 0, 0, 0, 0 },                       // 2b
	{ "ret2", 0, 0, 0, 0 },                 // 2c
	{ "retv", 0, 0, 1, 0 },                 // 2d
	{ "initloop", 0, 0, 0, 0 },             // 2e
	{ "addsv", 2, op_varref, 0, 0 },                // 2f
	{ "in", 0, 0, 2, 1 },                       // 30
	{ "conv_something", 4, op_immed_and_relative_jump, 0, 0 },          // 31
	{ "retz", 0, 0, 0, 0 },                  // 32
	{ "say", 0, 0, 0, 0 },                  // 33
	{ NULL, 0, 0, 0, 0 },                       // 34
	{ NULL, 0, 0, 0, 0 },                       // 35
	{ NULL, 0, 0, 0, 0 },                       // 36
	{ NULL, 0, 0, 0, 0 },                       // 37
	{ "callis", 3, op_call, 0, 1 },                 // 38
	{ "calli", 3, op_call, 0, 0 },                  // 39
	{ NULL, 0, 0, 0, 0 },                       // 3a
	{ NULL, 0, 0, 0, 0 },                       // 3b
	{ NULL, 0, 0, 0, 0 },                       // 3c
	{ NULL, 0, 0, 0, 0 },                       // 3d
	{ "push\titemref", 0, 0, 0, 1 },                // 3e
	{ "abrt", 0, 0, 0, 0 },                 // 3f
	{ "endconv", 0, 0, 0, 0 },                  // 40
	{ NULL, 0, 0, 0, 0 },                       // 41
	{ "pushf", 2, op_flgref, 0, 1 },                    // 42
	{ "popf", 2, op_flgref, 1, 0 },                 // 43
	{ "pushb", 1, op_byte, 0, 1 },                  // 44
	{ NULL, 0, 0, 0, 0 },                       // 45
	{ "setarrayelem", 2, op_immed, 2, 0 },          // 46
	{ "calle", 2, op_funid, 1, 0 },                 // 47
	{ "push\teventid", 0, 0, 0, 1 },                // 48
	{ NULL, 0, 0, 0, 0 },                       // 49
	{ "arra", 0, 0, 2, 1 },                 // 4a
	{ "pop\teventid", 0, 0, 1, 0 },                 // 4b
	{ "dbgline", 2, op_immed, 0, 0 },                  // 4c
	{ "dbgfunc", 4, op_data_string, 0, 0 },            // 4d
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },           // 4e - 4f
	{ "push\tstatic", 2, op_staticref, 0, 1 },          // 50
	{ "pop\tstatic", 2, op_staticref, 1, 0 },           // 51
	{ "callo", 2, op_funid, 1, 1 },             // 52
	{ "callind", 0, op_funcall, 2, 0 },         // 53
	{ "push\tclsvar", 2, op_classvarref, 0, 1 },            // 54
	{ "pop\tclsvar", 2, op_classvarref, 1, 0 },         // 55
	{ "callm", 2, op_clsfun, 0, 0 },                    // 56
	{ "callms", 4, op_clsfun_vtbl, 0, 0 },  // 57
	{ "clscreate", 2, op_clsid, 0, 0 }, // 58
	{ "classdel", 0, 0, 1, 0 }, // 59
	{ "aidxs", 2, op_staticref, 1, 1 }, // 5a
	{ "setstaticarrayelem", 2, op_immed, 2, 0 },    // 5b
	{ "staticloop", 10, op_static_sloop, 0, 0 },    // 5c
	{ "aidxclsvar", 2, op_classvarref, 1, 1 }, // 5d
	{ "setclsvararrayelem", 2, op_immed, 2, 0 },    // 5e
	{ "clsvarloop", 10, op_sloop, 0, 0 },    // 5f
	{ "push\tchoice", 0, 0, 0, 1 },                   // 60
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 61-63
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 64-67
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 68-6b
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 6c-6f
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 70-73
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 74-77
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 78-7b
	{ NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   { NULL, 0, 0, 0, 0 },   // 7c-7f
	{ NULL, 0, 0, 0, 0 },                       // 80
	{ NULL, 0, 0, 0, 0 },                       // 81
	{ "loop32", 12, op_sloop32, 0, 0 },       // 82
	{ NULL, 0, 0, 0, 0 },                       // 83
	{ "startconv32", 4, op_relative_jump32, 0, 0 }, // 84
	{ "jne32", 4, op_relative_jump32, 1, 0 },       // 85
	{ "jmp32", 4, op_uncond_jump32, 0, 0 },     // 86
	{ "cmps32", 6, op_argnum_reljump32, 0, 0 }, // 87
	{ NULL, 0, 0, 0, 0 },                       // 88
	{ NULL, 0, 0, 0, 0 },                   // 89
	{ NULL, 0, 0, 0, 0 },                   // 8a
	{ NULL, 0, 0, 0, 0 },                   // 8b
	{ NULL, 0, 0, 0, 0 },                   // 8c
	{ NULL, 0, 0, 0, 0 },                   // 8d
	{ NULL, 0, 0, 0, 0 },                   // 8e
	{ NULL, 0, 0, 0, 0 },                       // 8f
	{ NULL, 0, 0, 0, 0 },                   // 90
	{ NULL, 0, 0, 0, 0 },                       // 91
	{ NULL, 0, 0, 0, 0 },               // 92
	{ NULL, 0, 0, 0, 0 },               // 93
	{ NULL, 0, 0, 0, 0 },           // 94
	{ NULL, 0, 0, 0, 0 },                       // 95
	{ NULL, 0, 0, 0, 0 },                   // 96
	{ NULL, 0, 0, 0, 0 },                   // 97
	{ NULL, 0, 0, 0, 0 },                   // 98
	{ NULL, 0, 0, 0, 0 },                   // 99
	{ NULL, 0, 0, 0, 0 },                   // 9a
	{ NULL, 0, 0, 0, 0 },                       // 9b
	{ "addsi32", 4, op_data_string32, 0, 0 },       // 9c
	{ "pushs32", 4, op_data_string32, 0, 1 },       // 9d
	{ NULL, 0, 0, 0, 0 },               // 9e
	{ "pushi32", 4, op_immed32, 0, 1 },             // 9f
	{ NULL, 0, 0, 0, 0 },                       // a0
	{ NULL, 0, 0, 0, 0 },               // a1
	{ NULL, 0, 0, 0, 0 },                   // a2
	{ NULL, 0, 0, 0, 0 },                       // a3
	{ "call32", 4, op_funid32, 0, 0 },     // a4
	{ NULL, 0, 0, 0, 0 },                   // a5
	{ NULL, 0, 0, 0, 0 },               // a6
	{ NULL, 0, 0, 0, 0 },                       // a7
	{ NULL, 0, 0, 0, 0 },                       // a8
	{ NULL, 0, 0, 0, 0 },                       // a9
	{ NULL, 0, 0, 0, 0 },                       // aa
	{ NULL, 0, 0, 0, 0 },                       // ab
	{ NULL, 0, 0, 0, 0 },                   // ac
	{ NULL, 0, 0, 0, 0 },                   // ad
	{ "initloop32", 0, 0, 0, 0 },              // ae
	{ NULL, 0, 0, 0, 0 },               // af
	{ NULL, 0, 0, 0, 0 },                       // b0
	{ "conv_something32", 6, op_immedreljump32, 0, 0 },         // b1
	{ NULL, 0, 0, 0, 0 },                   // b2
	{ NULL, 0, 0, 0, 0 },                   // b3
	{ NULL, 0, 0, 0, 0 },                       // b4
	{ NULL, 0, 0, 0, 0 },                       // b5
	{ NULL, 0, 0, 0, 0 },                       // b6
	{ NULL, 0, 0, 0, 0 },                       // b7
	{ NULL, 0, 0, 0, 0 },                   // b8
	{ NULL, 0, 0, 0, 0 },                   // b9
	{ NULL, 0, 0, 0, 0 },                       // ba
	{ NULL, 0, 0, 0, 0 },                       // bb
	{ NULL, 0, 0, 0, 0 },                       // bc
	{ NULL, 0, 0, 0, 0 },                       // bd
	{ NULL, 0, 0, 0, 0 },               // be
	{ NULL, 0, 0, 0, 0 },                   // bf
	{ NULL, 0, 0, 0, 0 },                   // c0
	{ NULL, 0, 0, 0, 0 },                       // c1
	{ "pushfvar", 0, 0, 1, 1 },                 // c2
	{ "popfvar", 0, 0, 2, 0 },                  // c3
	{ NULL, 0, 0, 0, 0 },                   // c4
	{ NULL, 0, 0, 0, 0 },                       // c5
	{ NULL, 0, 0, 0, 0 },           // c6
	{ "calle32", 4, op_funid32, 1, 0 },         // c7
	{ NULL, 0, 0, 0, 0 },               // c8
	{ NULL, 0, 0, 0, 0 },                       // c9
	{ NULL, 0, 0, 0, 0 },                   // ca
	{ NULL, 0, 0, 0, 0 },                   // cb
	{ NULL, 0, 0, 0, 0 },                   // cc
	{ "dbgfunc32", 8, op_data_string32, 0, 0 },    // cd
	{ NULL, 0, 0, 0, 0 },               // ce
	{ NULL, 0, 0, 0, 0 },                       // cf
	{ NULL, 0, 0, 0, 0 },                   // d0
	{ NULL, 0, 0, 0, 0 },                   // d1
	{ NULL, 0, 0, 0, 0 },                   // d2
	{ NULL, 0, 0, 0, 0 },                   // d3
	{ "callindex", 1, op_byte, 0, 0 },      // d4
	{ NULL, 0, 0, 0, 0 },                   // d5
	{ NULL, 0, 0, 0, 0 },                   // d6
	{ NULL, 0, 0, 0, 0 },                   // d7
	{ NULL, 0, 0, 0, 0 },               // d8
	{ NULL, 0, 0, 0, 0 },                       // d9
	{ NULL, 0, 0, 0, 0 },                   // da
	{ NULL, 0, 0, 0, 0 },                   // db
	{ "staticloop32", 12, op_sloop32, 0, 0 },       // dc
	{ NULL, 0, 0, 0, 0 },                       // dd
	{ NULL, 0, 0, 0, 0 },                   // de
	{ "clsvarloop32", 12, op_sloop32, 0, 0 }       // df

};



// Embedded function table

/*
 *  Tables of usecode intrinsics:
 */
#define USECODE_INTRINSIC_PTR(NAME) #NAME

const char *bg_intrinsic_table[] = {
#include "bgintrinsics.h"
};
const int bg_intrinsic_size = sizeof(bg_intrinsic_table) / sizeof(char *);
const char *si_intrinsic_table[] = {
#include "siintrinsics.h"
};
const int si_intrinsic_size = sizeof(si_intrinsic_table) / sizeof(char *);



#if 0
const char *func_table[] = {
	"get_random",                       // 0
	"execute_usecode_array",                // 1
	"delayed_execute_usecode_array",            // 2
	"show_npc_face",                    // 3
	"hide_npc_face",                    // 4
	"add_answer",                       // 5
	"remove_answer",                    // 6
	"push_answers",                     // 7
	"pop_answers",                      // 8
	"clear_answers",                    // 9
	"select_from_menu",                 // a
	"select_from_menu2",                    // b
	"input_numeric_value",                  // c
	"set_item_shape",                   // d
	"find_nearest",                     // e
	"play_sound_effect",                    // f
	"die_roll",                     // 10
	"get_item_shape",                   // 11
	"get_item_frame",                   // 12
	"set_item_frame",                   // 13
	"get_item_quality",                 // 14
	"set_item_quality",                 // 15
	"get_item_quantity",                    // 16
	"set_item_quantity",                    // 17
	"get_object_position",                  // 18
	"get_distance",                     // 19
	"find_direction",                   // 1a
	"get_npc_object",                   // 1b
	"get_schedule_type",                    // 1c
	"set_schedule_type",                    // 1d
	"add_to_party",                     // 1e
	"remove_from_party",                    // 1f
	"get_npc_property",                 // 20
	"set_npc_property",                 // 21
	"get_avatar_ref",                   // 22
	"get_party_list",                   // 23
	"create_new_object",                    // 24
	"set_last_created",                 // 25
	"update_last_created",                  // 26
	"get_npc_name",                     // 27
	"count_objects",                    // 28
	"take_from_owner",                  // 29
	"get_container_items",                  // 2a
	"remove_party_items",                   // 2b
	"add_party_items",                  // 2c
	NULL,                           // 2d
	"play_music",                       // 2e
	"npc_nearby",                       // 2f
	"find_nearby_avatar",                   // 30
	"is_npc",                       // 31
	"display_runes",                    // 32
	"click_on_item",                    // 33
	NULL,                           // 34
	"find_nearby",                      // 35
	"give_last_created",                    // 36
	"is_dead",                      // 37
	"game_hour",                        // 38
	"game_minute",                      // 39
	"get_npc_number",                   // 3a
	"part_of_day",                      // 3b
	"get_alignment",                    // 3c
	"set_alignment",                    // 3d
	"move_object",                      // 3e
	NULL,                           // 3f
	"item_say",                     // 40
	"projectile_effect",                    // 41
	"get_lift",                     // 42
	"set_lift",                     // 43
	NULL,                           // 44
	NULL,                           // 45
	"sit_down",                     // 46
	NULL,                           // 47
	"display_map",                      // 48
	"kill_npc",                     // 49
	NULL,                           // 4a
	"set_npc_attack_mode",                  // 4b
	"set_target_npc_to_attack",             // 4c
	"clone_npc",                        // 4d
	NULL,                           // 4e
	"show_crystal_ball",                    // 4f
	"show_wizard_eye",                  // 50
	"resurrect_npc",                    // 51
	"add_spell",                        // 52
	"sprite_effect",                    // 53
	NULL,                           // 54
	"book_mode",                        // 55
	"stop_time",                        // 56
	"cause_light",                      // 57
	"get_barge",                        // 58
	"earthquake",                       // 59
	"is_player_female",                 // 5a
	"armageddon",                       // 5b
	"halt_scheduled",                   // 5c
	"cause_blackout",                   // 5d
	"get_array_size",                   // 5e
	"mark_stone",                       // 5f
	"recall_stone",                     // 60
	NULL,                           // 61
	"is_pc_inside",                     // 62
	"set_orrery_state",                 // 63
	NULL,                           // 64
	"get_timer",                        // 65
	"set_timer",                        // 66
	NULL,                           // 67
	"mouse_exists",                     // 68
	NULL,                           // 69
	"flash_mouse",                      // 6a
	NULL,                           // 6b
	NULL,                           // 6c
	NULL,                           // 6d
	"get_container",                    // 6e
	"remove_item",                      // 6f
	NULL,                           // 70
	NULL,                           // 71
	"get_equipment_list",                   // 72
	NULL,                           // 73
	NULL,                           // 74
	"start_endgame",                    // 75
	"fire_cannon",                      // 76
	"nap_time",                     // 77
	"advance_time",                     // 78
	"in_usecode",                       // 79
	NULL,                           // 7a
	NULL,                           // 7b
	NULL,                           // 7c
	"path_run_usecode",                 // 7d
	"close_gumps",                      // 7e
	"item_say",                     // 7f
	NULL,                           // 80
	"in_gump_mode",                     // 81
	NULL,                           // 82
	NULL,                           // 83
	NULL,                           // 84
	"is_not_blocked",                   // 85
	NULL,                           // 86
	"direction_from",                   // 87
	"get_npc_flag",                     // 88
	"set_npc_flag",                     // 89
	"clear_npc_flag",                   // 8a
	"run_usecode",                      // 8b
	"fade_palette",                     // 8c
	"get_party_list2",                  // 8d
	"in_combat",                        // 8e
	NULL,                           // 8f
	NULL,                           // 90
	NULL,                           // 91
	NULL,                           // 92
	NULL,                           // 93
	NULL,                           // 94
	NULL,                           // 95
	NULL                            // 96
};
#endif

#endif
