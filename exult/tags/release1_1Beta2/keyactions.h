/*
 *  Copyright (C) 2001 The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef KEYACTIONS_H
#define KEYACTIONS_H

void ActionQuit(int* params);
void ActionFileGump(int* params);
void ActionQuicksave(int* params);
void ActionQuickrestore(int* params);
void ActionAbout(int* params);
void ActionHelp(int* params);
void ActionCloseGumps(int* params);
void ActionCloseOrMenu(int* params);
void ActionMenuGump(int* params);
void ActionOldFileGump(int* params);

void ActionScreenshot(int* params);
void ActionRepaint(int* params);
void ActionResIncrease(int* params);
void ActionResDecrease(int* params);
void ActionBrighter(int* params);
void ActionDarker(int* params);
void ActionFullscreen(int* params);

void ActionUseItem(int* params);
void ActionUseFood(int*params);
void ActionCombat(int* params);
void ActionCombatPause(int* params);
void ActionTarget(int* params);
void ActionInventory(int* params);
void ActionTryKeys(int* params);
void ActionStats(int* params);
void ActionCombatStats(int* params);
void ActionFaceStats(int* params);

void ActionSIIntro(int* params);
void ActionEndgame(int* params);
void ActionScrollLeft(int* params);
void ActionScrollRight(int* params);
void ActionScrollUp(int* params);
void ActionScrollDown(int* params);
void ActionWalkWest(int* params);
void ActionWalkEast(int* params);
void ActionWalkNorth(int* params);
void ActionWalkSouth(int* params);
void ActionWalkNorthEast(int* params);
void ActionWalkSouthEast(int* params);
void ActionWalkNorthWest(int* params);
void ActionWalkSouthWest(int* params);
void ActionStopWalking(int* params);
void ActionCenter(int* params);
void ActionShapeBrowser(int* params);
void ActionCreateShape(int* params);
void ActionDeleteObject(int* params);
void ActionDeleteSelected(int *params);
void ActionMoveSelected(int *params);
void ActionToggleEggs(int* params);
void ActionGodMode(int* params);
void ActionGender(int* params);
void ActionCheatHelp(int* params);
void ActionInfravision(int* params);
void ActionSkipLift(int* params);
void ActionLevelup(int* params);
void ActionMapEditor(int* params);
void ActionHackMover(int* params);
void ActionMapTeleport(int* params);
void ActionTeleport(int* params);
void ActionTime(int* params);
void ActionWizard(int* params);
void ActionHeal(int* params);
void ActionCheatScreen(int* params);
void ActionPickPocket(int *params);
void ActionNPCNumbers(int *params);
void ActionGrabActor(int *params);

void ActionCut(int *params);
void ActionCopy(int *params);
void ActionPaste(int *params);

void ActionPlayMusic(int* params);
void ActionNaked(int* params);
void ActionPetra(int* params);
void ActionSkinColour(int* params);
void ActionSoundTester(int* params);
void ActionTest(int* params);

#endif
