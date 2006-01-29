========================================================
Keyring Mod Readme
========================================================

0	Table of Contents
---------------------
	0	Table of Contents
	1	About this document
	2	Installing the mod
	3	Contributing to this mod
	4	Using it in your own mods
	5	What this mod does
	6	Spoilers
	7	Version history


1	About this document
-----------------------
Author: Marzo Sette Torres Junior
e-mail: marzojr@taskmail.com.br
version: 0.10.02

	This mod (the so called "Keyring Mod") contains a lot of small
	modifications to the Ultima 7: Black Gate game WITH the Forge
	of Virtue add-on. There are also a number of not-so-small changes.
	There is a lot of original usecode and art in the mod too.
	
	There are many acknowledgements I have to make. One is for Team
	Lazarus; I borrowed some ideas for the Shrines and Codex from
	them. The other acknowledgements are to those that gave me some
	useful suggestions in the old Phorum thread. They are Crysta the Elf,
	gruck, Gradilla Dragon, Dominus and Artaxerxes.
	
	I must also thank Crysta the Elf for some graphics he made/edited.
	The full list is in the version history, below.

	Also, the structure of the mod is based on Alun Bestor's Quests and
	Iteractions mod.

2	Installing the mod
----------------------
	First, I must be explicit that I have tested this mod *only* with
	Forge of Virtue add-on installed; it will *not* work without that
	add-on since it uses some of the add-on's data.
	Secondly (and just as importantly), this mod is will *not* work in
	Exult 1.2, nor it will work on any but *the* latest snapshots. There
	were some bugs I had to fix to get everything working as is.
	
	With that out of the way: there are two ways to install the mod:
	
	1) If you are using DOS/Windows, you can use the supplied batch files.
	   Unzip the mod to your Black Gate folder (so that, for example,
	   the "Install.bat" file is at "BlackGate/Keyring/Install.bat").
	   Make sure that the mod's folder is at the same level as your
	   STATIC folder! Then run the "Install.bat" file to install. You 
	   can also run "Make & Install.bat" to compile the usecode and
	   install -- but you must have UCC to do so, and at a very
	   specific location too.
	2) Copy the contents of the "data" folder to your BG's PATCH folder.
	
	OPTIONAL: If you like, you can set new key bindings to BG to use the
	keyring with a keyboard shortcut. You will have to edit the file
	"bgdefaultkeys.txt". If you want to make it work like SI does,
	delete the line "K		try_keys" and add the following two lines:
		Alt-K		try_keys
		K		useitem			1100	# Use keyring
	
	
3	Contributing to this mod
----------------------------
	If you have any bugs that you would like to see fixed -- or that you
	*have* fixed -- please send it to me! Use the e-mail above.

4	Using it in your own mods
-----------------------------
	You are free to use anything I have written in your own mods;
	I ask only that you give me proper credit -- and maybe tell me
	about the mod you are doing, as I might be interested enough to
	help and/or to include as a part of this mod.

5	What this mod does
----------------------
	The mod started out by adding a keyring to Black Gate. It has two
	new NPCs, several new items and graphics and the following things:
	- You can now meditate at the Shrines of the Virtues.
	- You can now view the Codex; you must be in a sacred quest, though.
	- A brand new Shrine of the Codex, based on the one at the Mysterious
	  Sosaria website.
	- The three items of Principles (Book of Truth, Candle of Love and
	  Bell of Courage), as well as the three Flames of Principles.
	- Lock Lake Cleanup: Once Miranda's bill is signed, the Lake will
	  be gradually cleaned up of garbage.
	- A brand new Shrine of Spirituality and a basement for Serpent's Hold,
	  where the Flame of Courage is located. Both are located in a new map,
	  thus using Exult's Multimap support.
	- An improved Orb of the Moons, allowing you to visit the shrines too.
	- Innkeepers will reclaim the room keys when you are leaving the inn.
	  They will also lock the doors and make the beds.

6	Spoilers
------------
	So, you can't keep away... very well. Here they are. Proceed at your
	own peril...
	
	Keyring Spoilers
	----------------
		- The Quest is started by Zauriel. He is located just outside of
		  Lord British's castle. You can't miss him :-)
		- Most dialogs have a 'shortcut' out with no ill effects.
		- You must be very powerful to survive the quest.
		- Try hackmoving Laurianna to Zauriel without giving her some
		  blackrock first. Then talk to Zauriel.
		- It is Laurianna that will give you the keyring. She will only
		  do so *after* you give her Zauriel's Journal.
		- Using the keyring on the Avatar causes all party keys to be
		  added to it.
	
	Shrines of Virtues
	------------------
		- The Shrine of Sacrifice starts out defiled; you can restore it
		  with the correct Word of Power.
		- Before cleasing the Shrine of Sacrifice, look for the "Book of
		  Forgotten Mantras" and take it with you. When you choose to say
		  a Mantra, you can also select those from the book.
		- After you meditate for the right amount of cycles and with the
		  right mantra, each shrine will give you a quest to go see the
		  Codex of Ultimate Wisdom.
		- After going through seven shrines, and if you don't have access
		  to the Shrine of Spirituality, you will be able to attune the
		  white Virtue Stone to the Shrine with the Codex. Be sure to
		  have a way out, or you may be trapped!
		- Returning to the Shrine after seeing the Codex gives you a reward.
	
	Shrine of the Codex
	-------------------
		- You can only enter if you are in a sacred quest.
		- You must have both lenses to view the Codex. You can double-click
		  the lenses to rotate them.
		- Be sure to carry the lenses with you, or they may be trapped inside
		  the shrine if you can no longer enter!
		- Returning the Items of Principles to their original places is the
		  right thing to do.
		- Try teleporting inside the Shrine and viewing the Codex when you are
		  not on a quest to see it.
	
	Improved Orb of the Moons
	-------------------------
		- You can now teleport to the Shrines too; just throw the orb far
		  from you. Here are the new destinations of the Orb:
		Honesty							Compassion					Valor
		
						Moonglow		Britain			Jhelom
						
		Humility		New Magincia	(Avatar)		Yew			Justice
		
						Skara Brae		Trinsic			Minoc
						
		Spirituality					Honor						Sacrifice
	
	Lock Lake Cleanup
	-----------------
		- Eventually, Mack's key will be in possession of Lord Heather. You
		  can ask him for it.
	
7	Version history
-------------------
version: 0.10.02
	- Some faces were retouched by Crysta the Elf: Zauriel, Laundo and
	  Joneleth.
	- The Codex book displayed in-game got its own graphic, instead of
	  reusing Zauriel's Journal, made by Crysta the Elf.
	- New graphic for Book of Truth by Crysta the Elf (slightly edited
	  by me).
	- 'Wild' wisps weren't displaying their portrait.
	- Shamino was identifying the 'Shrine of the Codex' as being the
	  'Shrine of Humility'. Some ranger :-)
	- Added some feedback if the player is leaving the Shrine of the
	  Codex *after* finishing all quests but is leaving the lenses,
	  the Items of Principle and/or the Vortex Cube.
version: 0.10.01
	- Fixed initgame.dat and install.bat.
version: 0.10.00
	- Document created.