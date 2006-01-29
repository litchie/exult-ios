/*
 *	This source file contains a modified (and vastly improved, in organizational
 *	terms) usecode function for the blacksword.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

arcadionDialog 0x6F6 ()
{
	//Completelly reorganized and modularized this function. In the original, all of
	//the dialog is contained in a single function... I think things could still be
	//improved (e.g., by modifying the usecode for mirrors/gems/the black sword so that
	//they are the ones to call the dialog), but this is sufficient for the moment:
	
	if (!gflags[BROKE_MIRROR])
		arcadionMirrorFormDialog();
	
	else if (!gflags[COMMANDED_BOND])
	{
		if (event == DOUBLECLICK)
		{
			UI_close_gumps();
			script item after 1 ticks
				call arcadionGemFormDialog;
		}
		
		else if (event == SCRIPTED)
			arcadionGemFormDialog();
	}
	
	else
	{
		if (event == DOUBLECLICK)
		{
			UI_close_gumps();
			script item after 1 ticks
				call arcadionSwordFormDialog;
		}
		
		else if (event == SCRIPTED)
			arcadionSwordFormDialog();
	}
}
