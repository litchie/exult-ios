//This is to prevent the Test of Purity bug in Serpent Fang.

Pillar shape#(0x2AF) ()
{
	if ((event == DOUBLECLICK) && (UI_get_item_frame(item) == 10) && 
			!pointInsideRect(UI_get_object_position(item), [0x741, 0x9E5], [0x746, 0x9EA]))
		return;
	else
		Pillar.original();
}
