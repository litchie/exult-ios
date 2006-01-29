/*
 *	This source file contains the usecode for the Blackrock potion.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Blackrock_Potion shape#(0x450) ()
{
	if ((event == DOUBLECLICK) && get_item_frame() ==0)
	{
		//Nothing happens if in incomplete form.
		//Get random party member to ask the Avatar if he is SURE:
		var npc = randomPartyMember();
		npc.say("@If thou dost remember it, Zauriel said this was a deadly poison. @Art thou sure thou dost want to use it?@");
		if (askYesNo())
		{
			//Alas, yes...
			npc.hide();
			//Ask for target:
			var target = UI_click_on_item();
			
			if (target->is_npc())
			{
				//An NPC:
				if (target->get_item_shape() == SHAPE_LAURIANNA_ROOTED)
				{
					//Fortunatelly, it is Laurianna
					avatarSpeak("As you approach Laurianna with the potion, she immediatelly begins talking to you. You ignore her protests and force the potion down her throat.");
					gflags[LAURIANNA_DRANK_POTION] = true;
					script item after 1 ticks call Laurianna;
				}
				else
				{
					//Someone else... DAMN YOU AVATAR!
					var npc_num = target->get_npc_number();
					avatarSpeak("In a terrible display of strength and brutality, you force the potion down your victim's throat. The effect is swift, and everyone nearby is shocked by your brutal display.");
					if (npc_num == BATLIN)
					{
						//Batlin is a special case, but he IS affected:
						UI_show_npc_face(0xFFE6, 0x0000);
						say("Batlin squirms and contorts for a while, and chokes @Damn thee, Avatar!@ very faintly. In what appears to be a spasm, Batlin makes a gesture and vanishes before your eyes.");
						gflags[0x00DA] = true;
						UI_remove_npc(UI_get_npc_object(0xFFE6));
					}
					else
					{
						//Simply kill anyone else:
						say("After contorting for a while, your poor victim stops squirming and lays dead at your feet.");
						target->killTargetNPC();
					}
				}
				//Destroy the potion:
				remove_item();
			}
		}
	}
}
