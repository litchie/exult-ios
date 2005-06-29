========================================================
Miscelaneous fixes to Serpent Isle Usecode
========================================================

0	Table of Contents
---------------------
	0	Table of Contents
	1	About this document
	2	Installing the mod
	3	Contributing to this mod
	4	Using it in your own mods
	5	What this mod does
	5	Version history


1	About this document
-----------------------
Author: Marzo Sette Torres Junior
e-mail: marzojr@taskmail.com.br
version: 0.15

	This document fixes a few of the Usecode bugs that plague Ultima VII: Serpent Isle.
I have tested it *only* with Silver Seed installed; while it *might* work without the
add-on, I woudn't count on it.

	A potion of the code (the Cantra healing part) is heavily based on Usecode available
with the Exult CVS code (it is the file located at "exult/content/si/usecode.uc"). I did
some minor tweaks and some reorganization to make it more compatible with the rest of 
the mod's usecode. The main reason I am including it here is that I don't think many
people would know about it otherwise... The Cantra cure was written by Jeff Freedman (aka
"DrCode"), and has been split into the files "npcs/cantra.uc" and "items/bucketcure.uc".

	Everything else has been my own work, although the structure of the mod is based on
Alun Bestor's Quests and Iteractions mod. Many fixes were based on the document found at
"exult/docs/usecode_bugs.txt" on the CVS snapshot.

2	Installing the mod
----------------------
	First, I must be explicit that I have tested this mod *only* with Silver Seed add-on
installed; I don't know if it will work or not without. It *might* work.
	Secondly (and just as importantly), this mod is only guaranteed to work on the latest
Exult snapshots. It will *not* work in Exult 1.2.
	
	With that out of the way: there are two ways to install the mod:
	
	1) If you are using DOS/Windows, you can use the supplied batch files. Unzip the mod
	   to your Serpent's Isle folder (so that, for example, the "Install.bat" file is at
	   "SerpentIsle/SI Fixes/Install.bat"). Make sure that the mod's folder is at the
	   same level as your STATIC folder! Then run the "Install.bat" file to install. You
	   can also run "Make & Install.bat" to compile the usecode and install -- but you
	   must have UCC to do so, and at a very specific location too.
	2) Copy the contents of the "data" folder to your SI's PATCH folder.
	
3	Contributing to this mod
----------------------------
	If you have any bugs that you would like to see fixed -- or that you *have* fixed --
please send it to me! I would love to add these to the mod, so that SI can be made even
better than it already is. Use the e-mail above.

4	Using it in your own mods
-----------------------------
	You are free to use anything I have written in your own mods; I ask only that you give
me proper credit -- and maybe tell me about the mod you are doing, as I might be interested
enough to help and/or to include as a part of this mod.

5	What this mod does
----------------------
	As the title implies, this mod fixes SI usecode bugs. The way that they are fixed does
not depend on starting a new game -- although if the save game is at a late enough part of
the game, you will not see some of the fixes.

	Here are the specific bugs that this mod addresses so far:

	- Cantra's healing.
	- If you prefer 24-hour time, you can have it. Find the following line in usecode.uc
	  and remove the starting double-slash:
			//#include "items/time_tellers.uc";
	  You will have to recompile the usecode afterwards, but it is not too hard.
	- After being cured of insanity, the Companions will thank the Avatar for it. They
	  will also join *before* Xenka is summoned.
	- Basement of Temple of Tolerance was mistakenly identified as "Temple of Logic" by
	  the "Locate" spell.
	- Gwenno can now receive the White Diamond Necklace from Iolo;
	- Gwenno will no longer try to talk to the Avatar when she is resurrected;
	- Shamino will add his own misplaced items in the exchanged item list when he joins;
	- On the same note, the moonsilk stockings and the filari also appear on that list;
	- Fixed a few bugs on the exchanged item list when you find out the origins of the
	  items. Specifically, the lab apparatus, the fur cap, the bear skull and the plain
	  shield had a few problems.
	- The Pillars in Serpent's Fang Keep no longer teleport you to the Test of Purity;
	- After the the Wall of Lights cutscene, you can no longer call Thoxa to "resurrect"
	  your possessed party. Or rather, you can call her but she won't bring Iolo, Dupre
	  and Shamino back as they are the Banes now. Even if you have their bodies right
	  there -- or elsewhere.
	- Dupre can no longer be resurrected after he sacrifices himself.
	- Iolo, Shamino and Dupre will all refuse to leave the party while you are on the
	  Spinebreaker Mountains. They will also refuse blue potions while there...
	- The Wall of Lights has been revamped also. There is nothing you can do now to keep
	  the companions out of it -- nothing I can think off, at any rate. Considering that
	  I *have* read the anti-walkthrough from http://www.it-he.org, that is a lot. Also,
	  the companions don't dump every item directly to the ground when they die -- items
	  dropped remain in their respective containers. Not really a bug, but was extremely
	  annoying...
	- Fixes the Monitor Banquet so that the pikeman from the training area will not
	  disappear anymore.
	- Ghosts no longer talk as the Chaos Hierophant when double-clicked.
	- Fawn Tower is properly cleaned up. Goblins no longer spawn after it is cleaned up
	  (unless the banes have been released) and the broken dishes all go away.
	- When you get Dupre's shield back from Luther, you now really give him his shield
	  back. The exchanged item list also registers that.
	- Iolo's lute is no longer duplicated by the Teleport Storm on Fawn -- a regular lute
	  appears instead.
	- Inn keys are now reclaimed by the innkeepers when you are leaving the inn. They will
	  lock the doors and make the beds too. Innkeepers will also drop inn keys when the
	  banes are released (or, in Simon's case, when he is slain).
	- The Vibrate spell no longer makes you drop the Usecode container, nor its contents.
	- The Firesnake spell finally *works* now! Tell me what you think of it. Just make
	  sure not to stand too close to your target...
	- Spells. In the original SI, there were no less than *four* "Ex Por" spells, and
	  a few other spells were out of synch with the manual. Specifically:
		Spell				Original SI			Manual/This Mod
		=========================================================
		Unlock Magic		Ex Por				Ex Por
		Erstam's Surprise	Ex Por				Ex Jux Hur*
		Vibrate				Ex Por				Uus Des Por Grav*
		Imbalance			Ex Por				Kal Vas An Grav**
		Create Ammo			In Hur Sanct		In Jux Ylem
		Create Ice			In Sanct Grav		In Frio
		Fetch				An Frio Xen Ex		Por Ylem
		Serpent Bond		An Frio Xen Ex		Kal Frio Xen Ex
		Stop Storm			Rel Hur				An Hur
	  
	    *	Thanks to Neutronium Dragon for the suggestions.
	    **	Perhaps "Kal Vas In Grav" is a better fit to the spell...
	    
6	Version history
-------------------
version: 0.15
	- Reorganized file structure.
	- Readme file had one omission: Basement of Temple of Tolerance was misidentified as
	  being "Temple of Logic" by "Locate" spell.
	- Cantra's healing now actually works...
	- If you prefer 24-hour time, you can have it. Find the following line in usecode.uc
	  and remove the starting double-slash:
			//#include "items/time_tellers.uc";
	  You will have to recompile the usecode afterwards, but it is not too hard.
	- After being cured of insanity, the Companions will thank the Avatar for it. They will
	  also join *before* Xenka is summoned.
	- Fixed a few bugs on the exchanged item list when you find out the origins of the
	  items. Specifically, the lab apparatus, the fur cap, the bear skull and the plain
	  shield had a few problems.
	- Fawn Tower is properly cleaned up. Goblins no longer spawn after it is cleaned up
	  (unless the banes have been released) and the broken dishes all go away.
	- When you get Dupre's shield back from Luther, you now really give him his shield
	  back. The exchanged item list also registers that.
	- Iolo's lute is no longer duplicated by the Teleport Storm on Fawn -- a regular lute
	  appears instead.
	- Inn keys are now reclaimed by the innkeepers when you are leaving the inn. They will
	  lock the doors and make the beds too. Innkeepers will also drop inn keys when the
	  banes are released (or, in Simon's case, when he is slain).
	- The Vibrate spell no longer makes you drop the Usecode container, nor its contents.
	- Slightly modified the Firesnake spell.
	- Spells. I took Neutronium Dragon's suggestions for Erstam's Surprise and Vibrate.	

