/*
 *	Patch for curing Cantra.
 *	Written: 9/20/03.
 *
 *	To use, you need a 'patch' directory alongside 'static' and 'gamedat'
 *	for SerpentIsle.  Then, inside 'patch', run:
 *		ucc -s -o usecode <this file>
 *	Start up game, and use water of logic on Cantra.
 */

const int avatar = -356;
const int cantra_id = -64;		// Cantra's NPC #.
const int CURED_CANTRA = 0x47;		// Flag when Cantra is cured.

const int talk_sched = 3;		// Talk schedule.
const int wait_sched = 15;

/*
 *	Cantra.  (I want this to override the original code for her.)
 */
Cantra 0x440()
	{
	var cantra = item;
	if (event != 1 || !UI_get_item_flag(cantra, 0x1e))
		{
		Cantra.original();
		return;
		}
	var i = UI_get_random(4);
	if (i == 1)
		cantra.say("I want thy flesh!");
	else if (i == 2)
		cantra.say("I want thy blood!");
	else if (i == 3)
		cantra.say("Blood! Blood everywhere!");
	else
		cantra.say("How hungry I am!");
	converse (["Hello, Cantra.", "Bye"])
		{
	case "Bye":
		break;
	case "Hello, Cantra." (remove):
		cantra.say("Cantra?");
		cantra.say("Was that my name?");
		add("Yes, you are Cantra!");
		add("Knight...");
	case "Yes, you are Cantra!" (remove):
		cantra.say("No. NO!  LIAR!");
		break;
	case "Knight..." (remove):
		cantra.say("Knight?");
		cantra.say("Perhaps... or was that someone else...");
		}
	}

/*
 *	This is called when the 'bucket' is used.
 */
Dump_bucket 0x88a(bucket)
	{
	var target = UI_click_on_item();
	if (!target)
		return;
	UI_printf(["The shape clicked on is %s", UI_get_item_shape(target)]);
	var cantra = target[1];		// It's item, position.
	if (cantra != UI_get_npc_object(cantra_id) ||
	    UI_get_item_quality(bucket) != 13 ||	// "Logic".
	    UI_get_item_frame(bucket) >= 6)
		{			// Let orig. function handle it.
		UI_set_intercept_item(target);
		Dump_bucket.original(bucket);
		return;
		}
	UI_set_schedule_type(cantra, wait_sched);
	UI_path_run_usecode([target[2], target[3], target[4]], 
		0xa00 /* Cure_cantra */, bucket, 10);
	}

/*
 *	Called when correct bucket is dumped on Cantra.
 *	Input:	Item = bucket.
 */
Cure_cantra 0xa00()
	{
	if (event != 10)
		return;			// Shouldn't happen.
	UI_close_gumps();
	UI_set_item_frame(item, 0);	// Now empty.
	var cantra = UI_get_npc_object(cantra_id);
	UI_obj_sprite_effect(cantra, 7, 0, 0, 0, 0, 0, 0);
	UI_clear_item_flag(cantra, 0x1e);	// No longer crazy.
	UI_set_schedule_type(cantra, talk_sched);
	gflags[CURED_CANTRA] = 1;	// We've done it.
	}
