/*
 *	Global flags, assuming 1000- are free.
 */
const int TALKED_DRCODE = 1000;
const int ASKED_ABOUT_PYRO = 1001;
const int ASKED_WHERE_PYRO = 1002;
const int ASKED_KNOW_PYRO = 1003;
const int LOST_FAQ = 1004;		// Talked to Dom about lost FAQ.
const int WILL_FIND_FAQ = 1005;

/*
 *	Item flags.  ++++These should be in an 'include' file.
 */
const int IF_MET = 28;

/*
 *	Existing functions in BG.
 */
extern Ask_yesno 0x90a();		// Returns true if 'Yes', false if 'No.

/*
 *	NPC #'s:
 */
const int AMY = 0x168;
const int AVATAR = -356;

/*
 *	Egg on island created just E. of Trinsic.
 */

new_island_egg0 0x740 ()
	{
					// Get random NPC (or Avatar).
	var party = UI_get_party_list();
	var party_cnt = UI_get_array_size(party);
	var npc;
	if (party_cnt > 1)		// Companions?
		npc = party[1 + UI_get_random(party_cnt - 1)];
	else
		npc = party[UI_get_random(party_cnt)];
	if (npc)
		{
		UI_show_npc_face(npc);
		say("Do I detect . . .~",
			". . . the smell of Usecode?");
		UI_remove_npc_face(npc);
		}
	}

/*
 *	'DrCode' on island.
 */
DrCode 0x564 ()
	{
	if (event == 0)
		{
		UI_item_say(item, "Someone say 'usecode'??");
		return;
		}
	else if (event != 1)
		return;
	UI_show_npc_face(item);
	var answers;
	if (gflags[TALKED_DRCODE])
		{
		say("I knew you would return!");
		answers = "How??";
		}
	else
		{
		say("Hello!  How may I help you?");
		gflags[TALKED_DRCODE] = true;
		answers = ["Name", "Job"];
		}
	converse ([answers, "Bye"])
		{
	case "Bye":
		break;
	case "Name" (remove):
		say("I'm DrCode");
	case "Job" (remove):
		say("I search...");
		UI_add_answer("Search for what?");
	case "Search for what?" (remove):
		say("... for Usecode!!");
		UI_add_answer("Usecode");
	case "How??" (remove):
		say("I felt a great disturbance in the Usecode...");
		if (gflags[WILL_FIND_FAQ] &&
		    UI_get_npc_object(AMY) in UI_get_party_list())
			{
			UI_show_npc_face(AMY);
			say("Say, Dr. Code...");
			say("You certainly have a lot of papers and books ",
						"strewn about");
			UI_show_npc_face(item);
			say("Er, yes, I suppose I do.");
			UI_show_npc_face(AMY);
			say("Are you sure the FAQ isn't somewhere amongst ",
							"them?");
			UI_remove_npc_face(AMY);
			say("Dr.Code looks away from Amy as he hides his ",
					"shaking hands in his pockets.");
			UI_add_answer("FAQ");
			}
	case "Usecode" (remove):
		say("Usecode is a mythical force.~",
		"Some believe it controls the fate of all in our world. ",
		"And to control it would give one ultimate power. ");
		UI_add_answer("Power");
	case "Power":
		say("It MUST not fall into evil hands.~",
				"I can not let it...~",
				"No!  NO!~", "NOOooooooo!!");
					// Put in combat/flee mode.
		UI_set_attack_mode(item, 7);
		UI_set_schedule_type(item, 0);
		break;
	case "FAQ" (remove):
		say("I don't know what you are talking about!");
		UI_show_npc_face(AMY);
		say("Are you sure?");
		UI_remove_npc_face(AMY);
		say("YES!  I can stand the guilt no more!");
		say("But... I only meant to borrow it.  I, er, hoped that it ",
			"would help me with my research into Usecode.");
		say("Dominik was asleep, and I planned to return it before he",
							" awoke");
		UI_show_npc_face(AVATAR);
		say("May we have it back then?");
		UI_remove_npc_face(AVATAR);
		say("I'm afraid to tell you...~ ...it's gone!");
		}
	UI_remove_npc_face(item);
	}

/*
 *	'Dominik' on island.
 */
Dominik 0x565 ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...xml");
		else if (n == 2)
			UI_item_say(item, "Damn mouse!");
		else if (n == 3)
			UI_item_say(item, "...sound font...");
		else if (n == 4)
			UI_item_say(item, "...data dir");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	say("I'm very busy, so please be brisk!");
	var answers;
	if (gflags[ASKED_ABOUT_PYRO])
		answers = ["Name", "Job", "Where is Pyro-X?"];
	else
		answers = ["Name", "Job"];
	answers = [answers, "Bye"];
	var faqcnt = 0;
	converse (answers)
		{
	case "Bye":
		break;
	case "Name" (remove):
		say("Dominik");
	case "Job":
		faqcnt = faqcnt + 1;
		if (faqcnt == 1)
			{
			say("Please see the FAQ.");
			UI_add_answer("FAQ");
			}
		else if (faqcnt == 2)
			say("I said:  See the FAQ.");
		else if (faqcnt >= 3)
			say("I've told you ", faqcnt - 1,
					" times already...~",
					"SEE THE FAQ!!!");
	case "FAQ" (remove):
		say("All of life's answers may be found there.~",
			"Though... there are some who think otherwise.");
		UI_add_answer("Otherwise?");
		if (!gflags[LOST_FAQ])
			UI_add_answer("May I see it?");
		else if (!gflags[WILL_FIND_FAQ])
			UI_add_answer("Find");
	case "Otherwise?" (remove):
		say("There is a crackpot who wanders this island.  ",
		"He calls himself 'DrCode', and mutters incessantly about ",
				"something called 'Usecode'");
		UI_add_answer("Usecode?");
	case "Usecode?" (remove):
		say("Don't bother me with that silly prattle! ",
				"I must work on the FAQ.~", "Begone!");
		break;
	case "May I see it?" (remove):
		gflags[LOST_FAQ] = true;
		say("Yes, of course.");
		say("You wait while he looks on his desk.");
		say("You wait a bit more while he hastily pulls out the drawers and peers in each.");
		say("Finally, he turns back, looking markedly upset.");
		say("It' gone!!  Where can it be?~",  
							"He turns to you.");
		say("Avatar!  It's vital that the FAQ be recovered.",
				"  Our users will be lost without it!");
		say("Would you help me recover it?");
		if (Ask_yesno())
			{
			gflags[WILL_FIND_FAQ] = true;
			say("Thank you, Avatar!",
					"  All Brittania awaits the recovery",
					" of this vital document.");
			}
		else
			say("Woe to us!  Our efforts are doomed.");
	case "Find" (remove):
		gflags[WILL_FIND_FAQ] = true;
		say("I KNEW you would reconsider.",
				"  Thank you, Avatar!");
	case "Where is Pyro-X?" (remove):
		say("As I wrote in the FAQ you have to ask Colourless!");
		UI_add_answer("Who is Colourless?");
		gflags[ASKED_WHERE_PYRO] = true;
	case "Who is Colourless?" (remove):
		say("Read the FAQ!!!");
		}
	UI_remove_npc_face(item);
	}
/*
 *	'Willem' on island.
 */
Willem 0x566 ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...Pentagram");
		else if (n == 2)
			UI_item_say(item, "...Pagan..");
		else if (n == 3)
			UI_item_say(item, "another world");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	say("Hello Avatar!");
		UI_add_answer("Name");
		UI_add_answer("Job");
		UI_add_answer("Bye");	
		converse
		{
		if (response == "Bye")
			break;
		else if (response == "Name")
			{
			say("I'm Willem");
			UI_remove_answer("Name");
			}
		else if (response == "Job")
			{
			say("I work on Pentagram!");
			UI_add_answer("What is Pentagram?");
			}
		else if (response == "What is Pentagram?")
			{
			say("Pentagram is a magical device that lets you travel to another world. ",
			"To the world of Pagan!");
			UI_remove_answer("What is Pentagram?");
			UI_add_answer("Tell me about Pagan");
			}
		else if (response == "Tell me about Pagan")
			{
			say("You will find out soon enough. Believe me!");
			UI_remove_answer("Tell me about Pagan");
			}
		}
	UI_remove_npc_face(item);
	}
/*
 *	'Fingolfin' on island.
 */
Fingolfin 0x567 ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...OSX");
		else if (n == 2)
			UI_item_say(item, "...Pentgram..");
		else if (n == 3)
			UI_item_say(item, "SDL");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	say("Hello Avatar!");
		UI_add_answer("Name");
		UI_add_answer("Job");
		UI_add_answer("Bye");	
		converse
		{
		if (response == "Bye")
			break;
		else if (response == "Name")
			{
			say("My name is Fingolfin");
			UI_remove_answer("Name");
			}
		else if (response == "Job")
			{
			say("I work on Pentagram!");
			UI_add_answer("What is Pentagram?");
			}
		else if (response == "What is Pentagram?")
			{
			say("You should ask Willem about this");
			UI_remove_answer("What is Pentagram?");
			}
		}
	UI_remove_npc_face(item);
	}
/*
 *	'Amy' on island.
 */
Amy 0x568 ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...Studio");
		else if (n == 2)
			UI_item_say(item, "...Time warp..");
		else if (n == 3)
			UI_item_say(item, "Information");
		return;
		}
	if (event != 1)
		return;
	if (!UI_get_item_flag(item, IF_MET))
		{			// First time.
		say("You see a young lady with blonde hair.",
			"  She looks a bit annoyed.");
		UI_show_npc_face(item);
		say("And what are you after?");
		}
	else
		{
		UI_show_npc_face(item);
		say("Hello again Avatar!");
		}
	var answers = "Name";
	var party = UI_get_party_list();
	if (item in party)
		answers = [answers, "Leave"];
	else
		answers = [answers, "Job"];
	answers = [answers, "Bye"];
	converse (answers)
		{
	case "Bye":
		break;
	case "Name" (remove):
		say("I'm Amy.");
	case "Job" (remove):
		say("I don't have a job. I'm just hanging out here and 
			 give information about the people on this island");
		UI_add_answer("What is the name of this Island?");
		UI_add_answer("Who lives on this Island?");
		UI_add_answer("Are you happy here?");
	case "What is the name of this Island?" (remove):
		say("It is called SourceForge Island. To remember the ",
			"smithy who provided them with their tools");
	case "Are you happy here?" (remove):
		say("It's okay...~", "...but I'd rather be more useful");
		converse (["Join", "Good luck"])
			{
		case "Join":
			if (gflags[WILL_FIND_FAQ])
				{
				say("Ah, yes!  I hear you are searching for",
				" something precious that was lost.",
				"  Perhaps I can prove to my fellow islanders",
				" that I'm not just a pretty face.");
				UI_add_to_party(item);
				}
			else
				{
				say("But you have not work for me!",
					"  All you do is wander aimlessly.");
				}
			break;
		case "Good luck":
			break;
			}
	case "Leave" (remove):
		UI_remove_from_party(item);
		say("Goodbye, for now, Avatar");
		break;
	case "Who lives on this Island?" (remove):
		say("Of whom do you want to speak?");
		converse ([	"Dr.Code",
				"Willem",
				"Colourless",
				"Fingolfin",
				"Darke",
				"Dominus",
				"EsBee-Ex",
				"Nobody" ])
			{
		case "Dr.Code" (remove):
			say("He's the master of this group. Unfortunately he got mad.");
		case "Willem" (remove):
			say("Some people refer to him as Arthuris Dragon or wjp. ",
			"He's working on some strange magical device.");
		case "Colourless" (remove):
			say("He's also a member of that mystical group called The Dragons. ",
			"Among other things he works with Willem on this magical device.");
		case "Fingolfin" (remove):
			say("If he's not working on other things he helps other members of this group.");
		case "Darke" (remove):
			say("Darke thinks he is a dangerous rabbit but he's just a cute bunny. ",
			"Unfortunately he is also getting slowly as mad as Dr.Code.");
		case "Dominus" (remove):
			say("Don't bother talking to him. ",
			"He's always refering to some book called FAQ which no one ever read.");
		case "EsBee-Ex" (remove):
			say("EsBee-Ex is a gargoyle. ",
				"He believes his twin brother is evil.");
		case "Nobody":
			break;
			}
		}
	UI_remove_npc_face(item);
	}

/*
 *	'Colourless' on island.
 */
Colourless 0x569 ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...glide");
		else if (n == 2)
			UI_item_say(item, "Winsockets?");
		else if (n == 3)
			UI_item_say(item, "Colorless?");
		else if (n == 4)
			UI_item_say(item, "...hack");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	say("You shouldn't be able to see me");
	if (gflags[ASKED_WHERE_PYRO])
		{
		UI_add_answer("Name");
		UI_add_answer("Job");		
		UI_add_answer("Do you know Pyro-X?");
		}
		else
		{
		UI_add_answer("Name");
		UI_add_answer("Job");
		}
		UI_add_answer("Bye");	
		converse
		{
		if (response == "Bye")
			break;
		else if (response == "Name")
			{
			say("I'm Colourless");
			UI_remove_answer("Name");
			}
		else if (response == "Job")
			{
			say("I work with Darke, Willem and Fingolfin on Pentagram.");
			UI_add_answer("Pentagram?");
			}
		else if (response == "Pentagram?")
			{
			say("You better ask Willem about this.");
			UI_remove_answer("Pentagram?");
			UI_add_answer("Willem?");
			}
		else if (response == "Willem?")
			{
			say("He's somewhere around here.");
			UI_remove_answer("Willem?");
			}
		else if (response == "Do you know Pyro-X?")
			{
			say("Yes, I know him. He's responsible for many explosions in Britania. ",
			"To hide this he calls himself EsBee-Ex and says all those crimes were
			 comitted by his so called Evil Twin,");
			gflags[ASKED_KNOW_PYRO] = true;
			UI_remove_answer("Do you know Pyro-X?");
		}
		}
	UI_remove_npc_face(item);
	}

/*
 *	'Darke' on island.
 */
Darke 0x56A ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...usecode");
		else if (n == 2)
			UI_item_say(item, "...xml..");
		else if (n == 3)
			UI_item_say(item, "bowfluff");
		else if (n == 4)
			UI_item_say(item, "...fluff");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	say("Beware of my sharp teeth!");
		UI_add_answer("Sharp teeth?");
		UI_add_answer("Name");
		UI_add_answer("Job");
		UI_add_answer("Bye");	
		converse
		{
		if (response == "Bye")
			break;
		else if (response == "Name")
			{
			say("I'm Darke");
			UI_remove_answer("Name");
			}
		else if (response == "Job")
			{
			say("I make USECODE");
			UI_add_answer("Usecode?");
			}
		else if (response == "Sharp teeth?")
			{
			say("I'm a cute bunny with sharp teeth.",
			"Don't anger me!");
			UI_remove_answer("Sharp teeth?");
			}
		else if (response == "Usecode?")
			{
			say("Ask that deranged Dr.Code");
			UI_remove_answer("Usecode?");
			UI_add_answer("Dr.Code?");
			}
		else if (response == "Dr.Code?")
			{
			say("He got mad because he looked at the evil Usecode!",
			"Don't believe what he says!");
			UI_remove_answer("Dr.Code");
			break;
			}
		}
	UI_remove_npc_face(item);
	}
/*
 *	'EsBee_Ex' on island.
 */
EsBee_Ex 0x56B ()
	{
	if (event == 0)
		{
		if (UI_get_schedule_type(item) == 14)
			{		// Sleeping.
			UI_item_say(item, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-4.
		if (n == 1)
			UI_item_say(item, "...burn");
		else if (n == 2)
			UI_item_say(item, "...Rome..");
		else if (n == 3)
			UI_item_say(item, "boom");
		else if (n == 4)
			UI_item_say(item, "...Pyro-X");
		return;
		}
	if (event != 1)
		return;
	UI_show_npc_face(item);
	if (gflags[ASKED_KNOW_PYRO])
		{
		say("Ah, did you find my twin?");
		UI_add_answer("You are Pyro-X!");
		}
	else
		{
 		say("Hello, did you see my evil twin?!");
		UI_add_answer("Evil twin?");
		UI_add_answer("Name");
		UI_add_answer("Job");
		UI_add_answer("Bye");
		}	
		converse
		{
		if (response == "Bye")
			break;
		else if (response == "Name")
			{
			say("I'm eesbee-eex");
			UI_remove_answer("Name");
			}
		else if (response == "Job")
			{
			say("I'm hiding from my evil twin");
			}
		else if (response == "Evil twin?")
			{
			say("Yes, my evil twin called Pyro-X.");
			UI_remove_answer("Evil twin?");
			UI_add_answer("Pyro-X");
			}
		else if (response == "Pyro-X")
			{
			say("I've never seen him before, ", 
			"but people blame him for all sorts of explosions and burnt houses. People say
			 it's me but I say it is my evil twin.");
			gflags[ASKED_ABOUT_PYRO] = true;
			UI_remove_answer("Pyro-X");			
			}
		else if (response == "You are Pyro-X!")
			{
			say("I'm NOT him! Go away! ",
			"Go away! Run! He's coming!");
			UI_remove_answer("Are you Pyro-X?");
			UI_explode(item, UI_find_nearest(item, 704, 20), 704);
			gflags[ASKED_ABOUT_PYRO] = false;
			gflags[ASKED_WHERE_PYRO] = false;
			gflags[ASKED_KNOW_PYRO] = false;
			break;
			}
		}
	UI_remove_npc_face(item);
	}

