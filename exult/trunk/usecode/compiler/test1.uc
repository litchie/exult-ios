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
	return c;
	}
