/*
 *	Global flags, assuming 1000- are free.
 */
enum Isleflags
	{
	TALKED_DRCODE = 1000,
	ASKED_ABOUT_PYRO,
	ASKED_WHERE_PYRO,
	ASKED_KNOW_PYRO,
	LOST_FAQ,			// Talked to Dom about lost FAQ.
	WILL_FIND_FAQ,
	RABBIT_TRACKS,
	CHURCH_CARROTS,
	RETURNED_FAQ
	};

/*
 *	Item flags.  ++++These should be in an 'include' file.
 */
const int IN_PARTY = 6;
const int IF_MET = 28;

/*
 *	Existing functions in BG.
 */
extern Ask_yesno 0x90a();		// Returns true if 'Yes', false if 'No.
extern Add_experience 0x911(var incr);	// Add exper. to each party member.

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
		npc->say("Do I detect . . .~",
			". . . the smell of Usecode?");
		npc->hide();
		}
	}

/*
 *	Egg behind church by the carrots.
 */

new_island_egg1 0x741 ()
	{
	if (!gflags[CHURCH_CARROTS])
		{
		AVATAR->say("Mmmm... They do make a tasty snack.");
		return;
		}
	if (!UI_get_item_flag(AMY, IN_PARTY))
		{
		AVATAR->say("There's something hidden here, but I cannot",
				" find it.  If only Amy were here.");
		return;
		}
					// Create book.
	var faq = UI_create_new_object(0x282);
	UI_set_item_quality(faq, 0x88);	// BOOK OF CIRCLES.  Good enough...
					// Place on top of egg.
	UI_update_last_created(UI_get_object_position(item));
	UI_remove_item(item);		// Done with this egg.
	AMY->say("Look!  There appears to be a book here!");
	AVATAR->say("Can it be...");
	AMY->say("Yes!  It is!  The lost FAQ!");
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
	var answers;
	if (gflags[TALKED_DRCODE])
		{
		item->say("I knew you would return!");
		answers = "How??";
		}
	else
		{
		item->say("Hello!  How may I help you?");
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
		add("Search for what?");
	case "Search for what?" (remove):
		say("... for Usecode!!");
		add("Usecode");
	case "How??" (remove):
		say("I felt a great disturbance in the Usecode...");
		if (gflags[WILL_FIND_FAQ] &&
		    UI_get_npc_object(AMY) in UI_get_party_list())
			{
			AMY->say("Say, Dr. Code...~",
			    "You certainly have a lot of papers and books ",
						"strewn about");
			item->say("Er, yes, I suppose I do.");
			AMY->say("Are you sure the FAQ isn't somewhere ",
							"amongst them?");
			AMY->hide();
			say("Dr.Code looks away from Amy as he hides his ",
					"shaking hands in his pockets.");
			add("FAQ");
			}
	case "Usecode" (remove):
		say("Usecode is a mythical force.~",
		"Some believe it controls the fate of all in our world. ",
		"And to control it would give one ultimate power. ");
		add("Power");
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
		AMY->say("Are you sure?");
		AMY->hide();
		say("YES!  I can stand the guilt no more!");
		say("But... I only meant to borrow it.  I, er, hoped that it ",
			"would help me with my research into Usecode.");
		say("Dominik was asleep, and I planned to return it before he",
							" awoke");
		AVATAR->say("May we have it back then?");
		AVATAR->hide();
		say("I'm afraid to tell you...~ ...it's gone!");
		}
	item->hide();
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
	item->say("I'm very busy, so please be brisk!");
	var answers;
	if (gflags[ASKED_ABOUT_PYRO])
		answers = ["Name", "Job", "Where is Pyro-X?"];
	else
		answers = ["Name", "Job"];
					// Find "FAQ" in party.
	var faq = UI_find_object(-357, 0x282, 0x88, -359);
	if (faq)
		answers = [answers, "Found the FAQ"];
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
			add("FAQ");
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
		add("Otherwise?");
		if (!gflags[LOST_FAQ])
			add("May I see it?");
		else if (!gflags[WILL_FIND_FAQ])
			add("Find");
	case "Otherwise?" (remove):
		say("There is a crackpot who wanders this island.  ",
		"He calls himself 'DrCode', and mutters incessantly about ",
				"something called 'Usecode'");
		add("Usecode?");
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
	case "Found the FAQ" (remove):
		AVATAR->say("Thanks to Amy's sharp eyes, we hath found",
							" the missing FAQ.");
		AVATAR->hide();
		UI_remove_item(faq);
		item->say("At last, I can rest, assured that our users' ",
				" questions shall be answered.");
		Add_experience(20);
		item->say("And, Avatar... I believe the presence of the FAQ",
			" may even have increased thy knowledge!");
		UI_set_npc_prop(AVATAR, 2, 3);	// Gain 3 intel. pts.
		gflags[RETURNED_FAQ] = true;
	case "Where is Pyro-X?" (remove):
		say("As I wrote in the FAQ you have to ask Colourless!");
		add("Who is Colourless?");
		gflags[ASKED_WHERE_PYRO] = true;
	case "Who is Colourless?" (remove):
		say("Read the FAQ!!!");
		}
	item->hide();
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
	item->say("Hello Avatar!");
		add("Name");
		add("Job");
		add("Bye");	
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
			add("What is Pentagram?");
			}
		else if (response == "What is Pentagram?")
			{
			say("Pentagram is a magical device that lets you travel to another world. ",
			"To the world of Pagan!");
			UI_remove_answer("What is Pentagram?");
			add("Tell me about Pagan");
			}
		else if (response == "Tell me about Pagan")
			{
			say("You will find out soon enough. Believe me!");
			UI_remove_answer("Tell me about Pagan");
			}
		}
	item->hide();
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
			UI_item_say(item, "...Pentagram..");
		else if (n == 3)
			UI_item_say(item, "SDL");
		return;
		}
	if (event != 1)
		return;
	item->say("Hello Avatar!");
		add("Name");
		add("Job");
		add("Bye");	
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
			add("What is Pentagram?");
			}
		else if (response == "What is Pentagram?")
			{
			say("You should ask Willem about this");
			UI_remove_answer("What is Pentagram?");
			}
		}
	item->hide();
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
	if (event == 2)			// Looking in bushes by Nadir.
		{
		item->say("Avatar!");
		AVATAR->say("Yes?  What hast thou found?");
		item->say("There are some strange marks on the ground.");
		item->say("Perhaps they were made by a rabbit.");
		gflags[RABBIT_TRACKS] = true;
		AVATAR->hide();
		item->hide();
		return;
		}
	if (event != 1)
		return;
	if (!UI_get_item_flag(item, IF_MET))
		{			// First time.
		say("You see a young lady with blonde hair.",
			"  She looks a bit annoyed.");
		item->say("And what are you after?");
		}
	else
		item->say("Hello again Avatar!");
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
		add("What is the name"*);
		add("Who lives"*);
		add("Are you happy here?");
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
	item->hide();
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
	item->say("You shouldn't be able to see me");
	if (gflags[ASKED_WHERE_PYRO])
		{
		add("Name");
		add("Job");		
		add("Do you know Pyro-X?");
		}
		else
		{
		add("Name");
		add("Job");
		}
		add("Bye");	
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
			add("Pentagram?");
			}
		else if (response == "Pentagram?")
			{
			say("You better ask Willem about this.");
			UI_remove_answer("Pentagram?");
			add("Willem?");
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
	item->hide();
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
	item->say("Beware of my sharp teeth!");
	add("Sharp teeth?");
	add("Name");
	add("Job");
	if (gflags[WILL_FIND_FAQ])
		add("FAQ");
	add("Bye");	
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
			add("Usecode?");
			}
		else if (response == "Sharp teeth?")
			{
			say("I'm a cute bunny with sharp teeth.",
			"Don't anger me!");
			UI_remove_answer("Sharp teeth?");
			}
		else if (response == "FAQ")
			{
			say("I'm frequently asked that question!");
			if (gflags[CHURCH_CARROTS])
				say("I've already told you about the ",
					"carrots.");
			else if (gflags[RABBIT_TRACKS] && 
					UI_get_item_flag(AMY, IN_PARTY))
				{
				AMY->say("We're those your tracks",
					" over by the shrine?");
				AMY->hide();
				item->say("Perhaps....");
				item->say("Am I in trouble?");
				AVATAR->say("Not if you help us.");
				item->say("I was looking for carrots, ",
					" but all I found was a discarded ",
					"book among the bushes.");
				item->say("It had lots of information, ",
					" but nothing about where to find ",
					"carrots.  So I didn't care ",
					"much for it.");
				AVATAR->say("Well, where is it now?");
				item->say("Uh, er, I, er, uh...");
				AVATAR->hide();
				item->say("...I don't know!",
					"  But I did find some nice carrots ",
					" behind that church.");
				gflags[CHURCH_CARROTS] = true;
				}
			}
		else if (response == "Usecode?")
			{
			say("Ask that deranged Dr.Code");
			UI_remove_answer("Usecode?");
			add("Dr.Code?");
			}
		else if (response == "Dr.Code?")
			{
			say("He got mad because he looked at the evil Usecode!",
			"Don't believe what he says!");
			UI_remove_answer("Dr.Code");
			break;
			}
		}
	item->hide();
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
	if (gflags[ASKED_KNOW_PYRO])
		{
		item->say("Ah, did you find my twin?");
		add("You are Pyro-X!");
		}
	else
		{
 		item->say("Hello, did you see my evil twin?!");
		add("Evil twin?");
		add("Name");
		add("Job");
		add("Bye");
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
			add("Pyro-X");
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
	item->hide();
	}

/*
 *	Nadir.
 */
Nadir 0x56C ()
	{
	var nadir = item;
	if (event == 0)
		{
		if (UI_get_schedule_type(nadir) == 14)
			{		// Sleeping.
			UI_item_say(nadir, "Zzzz...");
			return;
			}
		var n = UI_get_random(4);// 1-2.
		if (n == 1)
			UI_item_say(nadir, "...free software");
		else if (n == 2)
			UI_item_say(nadir, "...free beer");
		else if (n == 3)
			UI_item_say(nadir, "...autoconf");
		else if (n == 4)
			UI_item_say(nadir, "Follow the GPL!");
		return;
		}
	if (event != 1)
		return;
	say("A guard paces beside a shrine.  He stands at attention as you ",
						"approach.");
	nadir->say("Hail, traveller!");
	say("Hast thou come to study and meditate?");
	var answers = ["Name", "Job", "Study?"];
	if (gflags[WILL_FIND_FAQ])
		answers = [answers, "FAQ"];
	answers = [answers, "Bye"];
	converse(answers)
		{
	case "Bye":
		break;
	case "Name" (remove):
		say("My name is Nadir.  Are you a follower of the source?");
		add("Source?");
	case "Job" (remove):
		say("I guard the shrine of Free Software.  ",
			"All who come, rich or poor, may gaze upon the ",
			" source.");
		add("Source?");
	case "Study?" (remove):
		say("Gaze upon the mirror... ",
			"The secrets of the universe dwell within!");
		AVATAR->say("Thanks, but maybe later.");
		nadir->say("Suit thyself.");
		AVATAR->hide();
	case "Source?" (remove):
		say("The source controls our fates.");
		if (gflags[TALKED_DRCODE])
			{
			AVATAR->say("But Dr.Code says Usecode does that.");
			nadir->say("That is so.");
			say("...and yet it is not.");
			say("...actually, it is but partly so.");
			say("But Dr.Code isn't quite right in the head.");
			say("He wanders around muttering, occasionally ",
				"dropping documents amongst the bushes.");
			AVATAR->hide();
			if (UI_get_npc_object(AMY) in UI_get_party_list())
				script AMY
					{
					wait 1;		step 6;
					wait 1;		step 6;
					wait 1;		frame 11;
					wait 1;		frame 12;
					wait 2;		frame 11;
					wait 1;		frame 0;
					wait 1;		call 0x568; //++++++
					}
			}
	case "FAQ" (remove):
		AVATAR->say("Hast thou happened to have seen the FAQ?");
		nadir->say("Others may deal with documentation, but I ",
				"concern myself only with the source.");
		AVATAR->hide();
		}
	nadir->hide();
	}
/*
 *	'Kirben' on island.
 */
Kirben 0x56D ()
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
			UI_item_say(item, "...speed");
		else if (n == 2)
			UI_item_say(item, "...thrift..");
		else if (n == 3)
			UI_item_say(item, "design");
		return;
		}
	if (event != 1)
		return;
	if (!UI_get_item_flag(item, IF_MET))		
		{			// First time.
		say("You see a scribe with many different scrolls around him.");
		item->say("Welcome to the Open Church of SourceForge Island!");
		}
	else
		item->say("Hello Avatar!");
	var answers;
	answers = ["Name", "Job"];
	answers = [answers, "Bye"];
	converse (answers)
	{
	case "Bye":
			break;
		case "Name" (remove):
			say("My name is Kirben.");
		case "Job" (remove):
			say("I am the official scribe of the Open Church of Sourceforge Island!");
			add("Sourceforge Island?");
			add("Open Church?");
			add("Scribe?");
		case "Sourceforge Island?" (remove):
			say("Amy knows all about our island and its inhabitants",
				"Ask her for more information.");
		case "Open Church?" (remove):
			say("In the Open Church of Sourceforge Island everyone ",
				"can preach his believes as long as he makes them freely availlable ", 
				"to everyone. ");
			say("Do you want to know more?");
			if (Ask_yesno())
				{
				say("Our church is based on three principles.");
				say("At the moment I have no idea what these are.",
					" I'm still waiting for someone to put some words in my mouth");
				}
			else
				say("Okay, come back later if you want to know more.");
		case "Scribe?" (remove):
			say("People rely on me to write down the changes in the beliefs",
				" every other day.");
			say("You can find the written beliefs of our people in the scrolls",
				" in this church.");
		}
	item->hide();
	}
/*
 *	'BillyG' on island.
 */
BillyG 0x56E ()
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
			UI_item_say(item, "...money");
		else if (n == 2)
			UI_item_say(item, "...security hole...");
		else if (n == 3)
			UI_item_say(item, "windows");
		return;
		}
	if (event != 1)
		return;
	if (!UI_get_item_flag(item, IF_MET))		
		{			// First time.
		say("You see a pathetic man with huge round glasses in his face.");
		item->say("Hello Avatar!");
		}
	else 
		item->say("Hello again!");
		var answers;
		answers = ["Name", "Job"];
		answers = [answers, "Bye"];
		converse (answers)
		{
		case "Bye":
			break;
		case "Name" (remove):
			say("My name is BillyG.");
		case "Job" (remove):
			say("At the moment I'm washing the windows of this island.");
			say("But soon I will own all these windows and no one may stop me.");
			say("Especially not that pathetic Open Church of fools.");
			add("Windows?");
			add("Open Church"*);
		case "Windows?" (remove):
			say("Windows rule the world.",
				" Without them we would live in darkness.");
			say("And soon I will own all the windows there are.");
			add("How?");
		case "Open Church of fools?" (remove):
			say("They stand in my way and try to resist me. ",
				"They think they can order me around to wash their windows but",
				" soon they will be mine."); 
		case "How?" (remove):
			say("Pretty simple. I'm going to save my money and ",
				" soon I'll have enough to buy every window on this island.");
		}
	item->hide();
	}
