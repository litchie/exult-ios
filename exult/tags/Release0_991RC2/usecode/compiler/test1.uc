#include "ucdefs.h"

extern adder 0x480(a, b);	// Declaration.
const int const13 = 13;

/*
 *	Return sum.
 */

adder1 0x481 (a, b)
	{
	var c;
	var d;
	string hello = "Hello, Avatar!";

	(-3)->adder(a, b);	// :-)
	c = a + b;
	d = [a, 1, b];		// Should pop as a, 1, b.
// 	hello = a;		// Should cause an error.
	converse
		{
		say("Hello, the time is ", 11, "o'clock");
		if (response == "Name")
			{
			say("My name is DrCode.");
			}
		else if (response == "Bye")
			return;
		}
				// This is nonsense:
	c = a[7];
	a[const13] = 46;
	c = item;
	event = event + 7;
	c = UI_get_item_flag(item, 10);
	c = UI_get_party_list();
	for(actor1 in c with i to max)
		{
		say("Hello, ", UI_get_npc_name(actor1));
		}
	var dd = a + 5, f, e = 2*d;
	return adder(a, 3);
	}

adder 0x480 (a, b)
	{
	return a + b;
	}

