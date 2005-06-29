//This removes the bug where all ghosts are Chaos Hierophants in disguise:

Ghost shape#(0x355) ()
{
	//If this is called via double-click, leave ASAP:
	if (event == DOUBLECLICK)
		return;
	//Otherwise, let the Chaos Hierophant say his bit:
	else
		Ghost.original();
}
