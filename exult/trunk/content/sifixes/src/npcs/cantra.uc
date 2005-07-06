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
	if (!get_item_flag(SI_ZOMBIE))
		Cantra.original();
	
	else
	{
		//Changed from Jeff's code for a SI-like behavior:
		if (event == DOUBLECLICK)
		{
			AVATAR->item_say("Hello, Cantra.");
			CANTRA->makePartyFaceNPC();
			delayedBark(CANTRA, "@I am not Cantra!@", 2);
			CANTRA->set_schedule_type(TALK);
		}
		else if (event == STARTED_TALKING)
		{
			CANTRA->run_schedule();
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
