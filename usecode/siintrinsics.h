/*
 *	Siintrinsics.h - Intrinsic table for Serpent Isle.
 *
 *	Note:	This is used in the virtual machine and the Usecode compiler.
 *
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	USECODE_INTRINSIC_PTR(get_random),			// 0
	USECODE_INTRINSIC_PTR(execute_usecode_array),		// 1
	USECODE_INTRINSIC_PTR(delayed_execute_usecode_array),	// 2

	USECODE_INTRINSIC_PTR(show_npc_face),	// Guess	// 3
	USECODE_INTRINSIC_PTR(remove_npc_face),	// Guess	// 4
	
	USECODE_INTRINSIC_PTR(show_npc_face0),			// 5
	USECODE_INTRINSIC_PTR(show_npc_face1),			// 6

	USECODE_INTRINSIC_PTR(remove_npc_face0), // Guess!	// 7
	USECODE_INTRINSIC_PTR(remove_npc_face1),		// 8
	USECODE_INTRINSIC_PTR(set_conversation_slot),		// 9
	USECODE_INTRINSIC_PTR(UNKNOWN),		// UNUSED	// 0xa
	USECODE_INTRINSIC_PTR(UNKNOWN),		// UNUSED	// 0xb

	USECODE_INTRINSIC_PTR(add_answer),			// 0xc
	USECODE_INTRINSIC_PTR(remove_answer),			// 0xd
	USECODE_INTRINSIC_PTR(push_answers),			// 0xe
	USECODE_INTRINSIC_PTR(pop_answers),			// 0xf
	USECODE_INTRINSIC_PTR(clear_answers),			// 0x10

	USECODE_INTRINSIC_PTR(select_from_menu),		// 0x11
	USECODE_INTRINSIC_PTR(select_from_menu2),		// 0x12
	USECODE_INTRINSIC_PTR(input_numeric_value),		// 0x13

	USECODE_INTRINSIC_PTR(set_item_shape),			// 0x14

	USECODE_INTRINSIC_PTR(find_nearest),			// 0x15
	USECODE_INTRINSIC_PTR(play_sound_effect),		// 0x16
	
	USECODE_INTRINSIC_PTR(die_roll),			// 0x17
	USECODE_INTRINSIC_PTR(get_item_shape),			// 0x18
	
	USECODE_INTRINSIC_PTR(get_item_usability),		// 0x19
	USECODE_INTRINSIC_PTR(get_item_frame),			// 0x1a
	USECODE_INTRINSIC_PTR(set_item_frame),			// 0x1b
	USECODE_INTRINSIC_PTR(get_item_quality),		// 0x1c
	USECODE_INTRINSIC_PTR(set_item_quality),		// 0x1d
	USECODE_INTRINSIC_PTR(get_item_quantity),		// 0x1e
	USECODE_INTRINSIC_PTR(set_item_quantity),		// 0x1f
	
	USECODE_INTRINSIC_PTR(get_object_position),		// 0x20
	USECODE_INTRINSIC_PTR(get_distance),			// 0x21
	USECODE_INTRINSIC_PTR(find_direction),			// 0x22
	USECODE_INTRINSIC_PTR(get_npc_object),			// 0x23
	USECODE_INTRINSIC_PTR(get_schedule_type),		// 0x24
	USECODE_INTRINSIC_PTR(set_schedule_type),		// 0x25
	USECODE_INTRINSIC_PTR(add_to_party),			// 0x26
	USECODE_INTRINSIC_PTR(remove_from_party),		// 0x27
	
	USECODE_INTRINSIC_PTR(get_npc_prop),			// 0x28
	USECODE_INTRINSIC_PTR(set_npc_prop),			// 0x29
	USECODE_INTRINSIC_PTR(get_avatar_ref),			// 0x2a
	USECODE_INTRINSIC_PTR(get_party_list),			// 0x2b
	
	USECODE_INTRINSIC_PTR(create_new_object),		// 0x2c - Known
	USECODE_INTRINSIC_PTR(create_new_object2),		// 0x2d

	USECODE_INTRINSIC_PTR(set_last_created),		// 0x2e 
	USECODE_INTRINSIC_PTR(update_last_created),		// 0x2f - Known
	USECODE_INTRINSIC_PTR(get_npc_name),			// 0x30
	USECODE_INTRINSIC_PTR(count_objects),			// 0x31
	USECODE_INTRINSIC_PTR(find_object),			// 0x32
	USECODE_INTRINSIC_PTR(get_cont_items),			// 0x33 - Appears correct
	USECODE_INTRINSIC_PTR(remove_party_items),		// 0x34
	USECODE_INTRINSIC_PTR(add_party_items),			// 0x35
	USECODE_INTRINSIC_PTR(add_cont_items),			// 0x36

	USECODE_INTRINSIC_PTR(remove_cont_items),		// 0x37 - SI Only ???
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x38 - Si Only
	USECODE_INTRINSIC_PTR(play_music),			// 0x39 - Known
	USECODE_INTRINSIC_PTR(npc_nearby),			// 0x3a - ???
	USECODE_INTRINSIC_PTR(npc_nearby),			// 0x3b
	USECODE_INTRINSIC_PTR(find_nearby_avatar),		// 0x3c

	USECODE_INTRINSIC_PTR(is_npc),				// 0x3d - Known
	USECODE_INTRINSIC_PTR(display_runes),			// 0x3e
	USECODE_INTRINSIC_PTR(click_on_item),			// 0x3f
	
	USECODE_INTRINSIC_PTR(error_message),			// 0x40 - Error Message

	USECODE_INTRINSIC_PTR(find_nearby),			// 0x41 - Known
	
	// Yes, these both seem to be 'give_last_created':
	USECODE_INTRINSIC_PTR(give_last_created),		// 0x42
	USECODE_INTRINSIC_PTR(give_last_created),		// 0x43

	USECODE_INTRINSIC_PTR(is_dead),				// 0x44

	USECODE_INTRINSIC_PTR(game_hour),			// 0x45 - Correct
	USECODE_INTRINSIC_PTR(game_minute),			// 0x46 - Correct

	USECODE_INTRINSIC_PTR(get_npc_number),			// 0x47 - Guess

	USECODE_INTRINSIC_PTR(part_of_day),			// 0x48
	USECODE_INTRINSIC_PTR(get_alignment),			// 0x49
	USECODE_INTRINSIC_PTR(set_alignment),			// 0x4a
	USECODE_INTRINSIC_PTR(move_object),			// 0x4b
	USECODE_INTRINSIC_PTR(remove_npc),			// 0x4c

	
	USECODE_INTRINSIC_PTR(item_say),			// 0x4d
	USECODE_INTRINSIC_PTR(clear_item_say),			// 0x4e

	USECODE_INTRINSIC_PTR(projectile_effect),		// 0x4f
	USECODE_INTRINSIC_PTR(get_lift),			// 0x50
	USECODE_INTRINSIC_PTR(set_lift),			// 0x51

	USECODE_INTRINSIC_PTR(get_weather),			// 0x52
	USECODE_INTRINSIC_PTR(set_weather),			// 0x53

	USECODE_INTRINSIC_PTR(sit_down),			// 0x54 - Known (is it???)

	// Packing
	USECODE_INTRINSIC_PTR(summon),				// 0x55
	USECODE_INTRINSIC_PTR(si_display_map),			// 0x56
	USECODE_INTRINSIC_PTR(kill_npc),			// 0x57
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x58
	USECODE_INTRINSIC_PTR(set_attack_mode),			// 0x59	
	USECODE_INTRINSIC_PTR(get_attack_mode),			// 0x5a
	USECODE_INTRINSIC_PTR(set_opponent),			// 0x5b
	// End pack


	USECODE_INTRINSIC_PTR(get_oppressor),			// 0x5c
	USECODE_INTRINSIC_PTR(get_weapon),			// 0x5d
	USECODE_INTRINSIC_PTR(set_oppressor),	// Guess	// 0x5e
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x5f UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x60 UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x61 UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x62 UNUSED
	USECODE_INTRINSIC_PTR(resurrect),	// Guess	// 0x63
	USECODE_INTRINSIC_PTR(UNKNOWN),			// 0x64
	USECODE_INTRINSIC_PTR(get_body_npc),			// 0x65
	USECODE_INTRINSIC_PTR(add_spell),			// 0x66
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x67 Remove all spells????
	USECODE_INTRINSIC_PTR(sprite_effect),			// 0x68
	USECODE_INTRINSIC_PTR(explode),				// 0x69 


	USECODE_INTRINSIC_PTR(book_mode),			// 0x6a - Known
	
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x6b
	USECODE_INTRINSIC_PTR(cause_light),			// 0x6c
	USECODE_INTRINSIC_PTR(get_barge),			// 0x6d
	USECODE_INTRINSIC_PTR(earthquake),			// 0x6e
	USECODE_INTRINSIC_PTR(is_pc_female),			// 0x6f - Known
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x70 UNUSED
	USECODE_INTRINSIC_PTR(halt_scheduled),			// 0x71
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x72  +++++CauseBlackout (ucdump.c)
	USECODE_INTRINSIC_PTR(get_array_size),			// 0x73
	
	USECODE_INTRINSIC_PTR(save_pos),			// 0x74
	USECODE_INTRINSIC_PTR(teleport_to_saved_pos),		// 0x75
	USECODE_INTRINSIC_PTR(apply_damage),			// 0x76
	USECODE_INTRINSIC_PTR(is_pc_inside),			// 0x77
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x78 UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x79 UNUSED
	USECODE_INTRINSIC_PTR(get_timer),			// 0x7a
	USECODE_INTRINSIC_PTR(set_timer),			// 0x7b
	USECODE_INTRINSIC_PTR(wearing_fellowship),		// 0x7c
	USECODE_INTRINSIC_PTR(mouse_exists),			// 0x7d
	USECODE_INTRINSIC_PTR(get_speech_track),		// 0x7e
	USECODE_INTRINSIC_PTR(flash_mouse),			// 0x7f
	USECODE_INTRINSIC_PTR(get_item_frame_rot),		// 0x80 Guess
	USECODE_INTRINSIC_PTR(set_item_frame_rot),		// 0x81 Guess
	USECODE_INTRINSIC_PTR(on_barge),			// 0x82 Guess
	USECODE_INTRINSIC_PTR(get_container),			// 0x83



	USECODE_INTRINSIC_PTR(remove_item),			// 0x84 - Known

	USECODE_INTRINSIC_PTR(init_conversation),		// 0x85 Guess
	USECODE_INTRINSIC_PTR(end_conversation),		// 0x86 Guess
	USECODE_INTRINSIC_PTR(reduce_health),			// 0x87 Guess
	USECODE_INTRINSIC_PTR(is_readied),			// 0x88
	USECODE_INTRINSIC_PTR(restart_game),			// 0x89
	USECODE_INTRINSIC_PTR(start_speech),			// 0x8a
	USECODE_INTRINSIC_PTR(run_endgame),			// 0x8b StartEndGame (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x8c UNUSED
	USECODE_INTRINSIC_PTR(nap_time),			// 0x8d
	USECODE_INTRINSIC_PTR(advance_time),			// 0x8e
	USECODE_INTRINSIC_PTR(in_usecode),			// 0x8f
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x90
	USECODE_INTRINSIC_PTR(obj_sprite_effect),		// 0x91
	USECODE_INTRINSIC_PTR(attack_avatar),			// 0x92

	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x93 ++++Stop_attacking_avatar()??

	USECODE_INTRINSIC_PTR(path_run_usecode),		// 0x94 - Known
	
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x95 UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x96
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x97 UNUSED
	USECODE_INTRINSIC_PTR(close_gumps),	// Guess	// 0x98
	USECODE_INTRINSIC_PTR(item_say),			// 0x99
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x9a
	USECODE_INTRINSIC_PTR(in_gump_mode),	// Guess	// 0x9b

	// Packing!!!
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x9c
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x9d UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x9e
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0x9f
	// End Pack

	USECODE_INTRINSIC_PTR(is_not_blocked),			// 0xa0
	
	USECODE_INTRINSIC_PTR(play_sound_effect2),		// 0xA1 - Known
	
	USECODE_INTRINSIC_PTR(direction_from),			// 0xa2
	
	USECODE_INTRINSIC_PTR(get_item_flag),			// 0xA3 - Known
	USECODE_INTRINSIC_PTR(set_item_flag),			// 0xA4 - Known
	USECODE_INTRINSIC_PTR(clear_item_flag),			// 0xA5 - Known
	
	USECODE_INTRINSIC_PTR(get_skin_colour),			// 0xa6
	USECODE_INTRINSIC_PTR(set_path_failure),		// 0xa7 
	USECODE_INTRINSIC_PTR(fade_palette),			// 0xa8
	// This one is used just for sleeping:
	USECODE_INTRINSIC_PTR(fade_palette),			// 0xa9
	USECODE_INTRINSIC_PTR(get_party_ids),			// 0xaa

	USECODE_INTRINSIC_PTR(in_combat),			// 0xab
	USECODE_INTRINSIC_PTR(is_water),			// 0xac
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xad UNUSED
	USECODE_INTRINSIC_PTR(set_camera),			// 0xae
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xaf UNUSED
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xb0 UNUSED
	USECODE_INTRINSIC_PTR(telekenesis),			// 0xb1
	USECODE_INTRINSIC_PTR(a_or_an),				// 0xb2 Return correct article for argument ('a' or 'an')
	USECODE_INTRINSIC_PTR(set_polymorph),		// 0xb3 Set Polymorph
	USECODE_INTRINSIC_PTR(revert_schedule),		// 0xb4 Reverts to static schedule
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xb5
	USECODE_INTRINSIC_PTR(set_new_schedules),		// 0xb6 Set New Schedules
	USECODE_INTRINSIC_PTR(run_schedule),			// 0xb7 Run the scheduled activity
	USECODE_INTRINSIC_PTR(get_temperature),			// 0xb8
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xb9 UNUSED
	USECODE_INTRINSIC_PTR(set_temperature),			// 0xba
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xbb UNUSED
	USECODE_INTRINSIC_PTR(get_npc_id),			// 0xbc - Known
	USECODE_INTRINSIC_PTR(set_npc_id),			// 0xbd - Known
	USECODE_INTRINSIC_PTR(get_readied),	// Guess	// 0xbe
//	USECODE_INTRINSIC_PTR(add_removed_npc),			// 0xbf
	USECODE_INTRINSIC_PTR(approach_avatar),			// 0xbf
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xc0
	USECODE_INTRINSIC_PTR(si_path_run_usecode),		// 0xc1
	USECODE_INTRINSIC_PTR(is_on_keyring),		// 0xc2
	USECODE_INTRINSIC_PTR(add_to_keyring),		// 0xc3
	USECODE_INTRINSIC_PTR(remove_from_area),		// 0xc4
	USECODE_INTRINSIC_PTR(infravision),			// 0xc5
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xc6
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xc7
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xc8
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xc9
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xca
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xcb
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xcc
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xcd
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xce
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xcf
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd0
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd1
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd2
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd3
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd4
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd5
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd6
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd7
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd8
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xd9
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xda
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xdb
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xdc
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xdd
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xde
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xdf
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe0
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe1
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe2
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe3
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe4
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe5
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe6
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe7
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe8
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xe9
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xea
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xeb
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xec
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xed
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xee
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xef
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf0
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf1
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf2
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf3
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf4
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf5
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf6
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf7
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf8
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xf9
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xfa
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xfb
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xfc
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xfd
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xfe
	USECODE_INTRINSIC_PTR(UNKNOWN),				// 0xff
