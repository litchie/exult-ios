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

	c = a + b;
	d = [a, 1, b];		// Should pop as a, 1, b.
// 	hello = a;		// Should cause an error.
	message("Hello, the time is ");
	message(11);
	message(" o'clock");
	say();
	if (UcResponse == "Name")
		{
		message("My name is DrCode.");
		say();
		}
	else if (UcResponse == "Bye")
		return;
				// This is nonsense:
	c = a[7];
	a[const13] = 46;
	c = UcItem;
	UcEvent = UcEvent + 7;
	c = UI_get_item_flag(UcItem, 10);
	c = UI_get_party_list();
	for (actor in c with i to max)
		{
		message("Hello, ");
		message(UI_get_npc_name(actor));
		}
	var dd = a + 5, f, e = 2*d;
	return adder(a, 3);
	}

adder 0x480 (a, b)
	{
	return a + b;
	}

