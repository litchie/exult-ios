Frigidazzi 0x414 ()
{
	if (gflags[KNOWS_FURCAP_OWNER])
	{
		//When you give the fur cap back to Frigidazzi,
		//she clears a flag she "shouldn't". This hack is
		//so that we don't have to rewrite her entire
		//usecode to fix that...
		//I ordinarily would simply cache the flag state,
		//call the original function and set the new flag...
		//but there are a couple hidden aborts when
		//Rotoluncia's automaton is *not* summoned which
		//prevents that...
		script AVATAR after 5 ticks
		{	nohalt;					call setFurcapFlag;}
	}
	
	Frigidazzi.original();
}
