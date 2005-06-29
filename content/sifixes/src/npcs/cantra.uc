/*
 *  This file has been created from usecode found in the Exult CVS snapshot.
 *  I include it here only for convenience; I have edited it to fit the
 *  conventions used in the rest of the mod.
 *
 *	The original code was written by Jeff Freedman (aka "DrCode").
 */

Cantra 0x440()
{
	//I prefer to check this flag instead:
	if (!UI_get_item_flag(item, SI_ZOMBIE))
		Cantra.original();
	
	else
	{
		//Changed from Jeff's code for a SI-like behavior:
		if (event == DOUBLECLICK)
		{
			UI_item_say(AVATAR, "Hello, Cantra.");
			CANTRA->makePartyFaceNPC();
			delayedBark(CANTRA, "@I am not Cantra!@", 2);
			UI_set_schedule_type(CANTRA, TALK);
		}
		else if (event == STARTED_TALKING)
		{
			UI_run_schedule(CANTRA);
			var msg = ["@I want thy flesh!@", "@I want thy blood!@", "@Blood! Blood everywhere!@", "@How hungry I am!@"];
			var rand = UI_get_random(UI_get_array_size(msg));
			item.say(msg[rand]);
			
			converse (["Hello, Cantra.", "bye"])
			{
				case "bye":
					say("@Yes, thou mayest go away.@");
					break;
		
				case "Hello, Cantra." (remove):
					say("@Cantra?@");
					say("@Was that my name?@");
					add("Yes, thou art Cantra!");
					add("knight...");
		
				case "Yes, thou art Cantra!" (remove):
					say("@No. NO!  LIAR!@");
					break;
		
				case "knight..." (remove):
					say("@Knight?@");
					say("@Perhaps... or was that someone else...@");
			}
		}
	}
}
