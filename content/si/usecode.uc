const int avatar = -356;
const int CURED_CANTRA = 0x47;		// Flag when Cantra is cured.

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
	UI_set_intercept_item(target);
	// Put code to detect dumping correct water on Cantra here.
	Dump_bucket.original(bucket);
	}
