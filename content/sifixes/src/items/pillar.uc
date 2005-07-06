//This is to prevent the Test of Purity bug in Serpent Fang.

Pillar shape#(0x2AF) ()
{
	if ((event == DOUBLECLICK) && (get_item_frame() == 10) && 
			!pointInsideRect(get_object_position(), [0x741, 0x9E5], [0x746, 0x9EA]))
		return;
	else
		Pillar.original();
}
