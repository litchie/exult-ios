#game "blackgate"
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
			say("My name is \"DrCode\".");
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
	c = item->get_item_flag(10);
	c = get_item_flag(10);
	c = UI_get_party_list();
	c = item->resurrect();
	for(actor1 in c with i to max)
		{
		say("Hello, ", UI_get_npc_name(actor1));
		}
	var dd = a + 5, f, e = 2*d;

	script item {
		nohalt;
		next frame;
	}

	return adder(a, 3);
	}

class Test
{
	var testvar;
	now_what() { ; }
	Fun2() { ; }
}

class Test2 : Test	// Inheritance: class Test2 has all data members and
		// functions of class Test, and can be converted to class Test if
		// needed by e.g., a function call.
{
	var testvar1;
	now_what1() { testvar1 = 0; Test::Fun2(); now_what(); Fun2(); }
	class<Test> Fun2()
	{
		testvar1 = 1;
		//return;	// Fails to compile
		//return 1;	// Fails to compile
		//return 0;	// Compiles correctly
		return this;	// this is of type class<Test2>, which is
				// derived from class<Test> hence can be
				// converted to it
	}
	Fun3()
	{
		//return 1;	// Fails to compile
		//return this;	// Fails to compile
		return;	// Fails to compile
	}
}

class Test3 : Test
{  }

class Test4 : Test2
{  }

class<Test3> Fun2 0xB00 (class<Test> a)
{
	class<Test> mytest;
	class<Test2> foo = new Test2(5, 6);	// Constructor example
	class<Test> bar = new Test2(7, 8);	// Constructor and casting example

	foo->Fun2();		// Calls Fun2 of class Test2 with 'foo' as 'this'
	foo->Test::Fun2();	// Calls Fun2 of class Test with 'foo' as 'this'
	foo->now_what1();

	bar = foo;
	//foo = bar;	// Fails to compile

	// Constructor and casting examples:
	mytest = new Test(9);
	mytest = new Test2(10, 11);
	mytest = new Test(12);
	mytest = new Test4(a, a);
	mytest = new Test3(a);
	bar = new Test(13);
	
	// Conditions:
	if (bar)
		;
		
	// Deletion:
	delete bar;
}

/*
adder 0x480 (a, b)
	{
	return a + b;
	}
*/
