#game "blackgate"
#include "ucdefs.h"

extern var adder 0xC00(a, b);	// Declaration.
const int const13 = 13;

/*
 *	Return sum.
 */

var adder1 0xC01 (a, b)
	{
	var c;
	var d;
	string hello = "Hello, Avatar!";

	adder(a, b);	// :-)
	c = a + b;
	d = [a, 1, b];		// Should pop as a, 1, b.
// 	hello = a;		// Should cause an error.
	say("Hello, the time is ", 11, "o'clock");
	// Old-style, super-verbose conversations:
	converse
		{
		if (response == "Name")
			{
			say("My name is \"DrCode\".");
			UI_remove_answer("Name");
			// This also works:
			// UI_remove_answer(user_choice);
			}
		else if (response == "Bye")
			break;
		else if (response in ["bogus", "nonsense", "garbage"])
			say("This is utter nonsense!");
		}
	say("Hello, the time is ", 12, "o'clock");
	// Newer conversation style:
	converse (["Name", "Bye", "name"])
		{
		case "Name" (remove):
			{
			say("My name is \"DrCode\".");
			}
		case "Bye":
			break;
		case "bogus", "nonsense", "garbage":
			say("This is utter nonsense!");
		default:
			say("All other answers get here. Your choice was ", user_choice);
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
	class<Test> Fun5()
	{
		testvar1 = 1;
		//return;	// Fails to compile
		//return 1;	// Fails to compile
		//return 0;	// Compiles correctly
		return this;	// this is of type class<Test2>, which is
				// derived from class<Test> hence can be
				// converted to it
	}
	Fun2() { ; }
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

class<Test> Fun2 0xB00 (class<Test> a)
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
	//mytest = new Test4(a, a);		// Fails to compile
	mytest = new Test4(1, 2);
	//mytest = new Test3(a);		// Fails to compile
	mytest = new Test3(3);
	bar = new Test(13);
	
	// Conditions:
	if (bar)
		;
		
	// Deletion:
	delete bar;
}

/*
var adder 0x480 (a, b)
	{
	return a + b;
	}
*/
