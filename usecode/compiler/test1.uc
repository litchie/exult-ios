//
//	Return sum.
//

adder 0x480 (a, b)
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
	return c;
	}
