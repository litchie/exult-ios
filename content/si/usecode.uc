const int avatar = -356;

/*
 *	Cantra.  (I want this to override the original code for her.)
 */
Cantra 0x440()
	{
	if (event != 1)
		return;
	var cantra = item;
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
