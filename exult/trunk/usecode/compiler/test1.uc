
extern adder 0x480(a, b);	// Declaration.

//
//	Return sum.
//

adder1 0x481 (a, b)
	{
	var c;
	var d;
	string hello = "Hello, Avatar!";

	c = a + b;
	d = [a, 1, b];		// Should pop as a, 1, b.
// 	hello = a;		// Should cause an error.
	UcMessage("Hello, the time is ");
	UcMessage(11);
	UcMessage(" o'clock");
	UcSay;
				// This is nonsense:
	c = a[7];
	a[13] = 46;
	c = UcItem;
	UcEvent = UcEvent + 7;
	c = UI_get_item_flag(UcItem, 10);
	c = UI_get_party_list();
	for (actor in c with i to max)
		{
		UcMessage("Hello, ");
		UcMessage(UI_get_npc_name(actor));
		}
	return adder(a, 3);
	}

adder 0x480 (a, b)
	{
	return a + b;
	}

