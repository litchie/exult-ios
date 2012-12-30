/*
 *  Copyright (C) 2001-2011 The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef KEYACTIONS_H
#define KEYACTIONS_H

void ActionQuit(int const *params);
void ActionFileGump(int const *params);
void ActionQuicksave(int const *params);
void ActionQuickrestore(int const *params);
void ActionAbout(int const *params);
void ActionHelp(int const *params);
void ActionCloseGumps(int const *params);
void ActionCloseOrMenu(int const *params);
void ActionMenuGump(int const *params);
void ActionOldFileGump(int const *params);
#ifdef UNDER_CE
void ActionMinimizeGame(int const *params);
void ActionTouchscreenMode(int const *params);
void ActionKeyboardPosition(int const *params);
void ActionKeyboardMode(int const *params);
#endif

void ActionScreenshot(int const *params);
void ActionRepaint(int const *params);
void ActionResIncrease(int const *params);
void ActionResDecrease(int const *params);
void ActionBrighter(int const *params);
void ActionDarker(int const *params);
void ActionFullscreen(int const *params);

void ActionUseItem(int const *params);
void ActionUseFood(int const *params);
void ActionCallUsecode(int const *params);
void ActionCombat(int const *params);
void ActionCombatPause(int const *params);
void ActionTarget(int const *params);
void ActionInventory(int const *params);
void ActionTryKeys(int const *params);
void ActionStats(int const *params);
void ActionCombatStats(int const *params);
void ActionFaceStats(int const *params);

void ActionSIIntro(int const *params);
void ActionEndgame(int const *params);
void ActionScrollLeft(int const *params);
void ActionScrollRight(int const *params);
void ActionScrollUp(int const *params);
void ActionScrollDown(int const *params);
int get_walking_speed(int const *params);
void ActionWalkWest(int const *params);
void ActionWalkEast(int const *params);
void ActionWalkNorth(int const *params);
void ActionWalkSouth(int const *params);
void ActionWalkNorthEast(int const *params);
void ActionWalkSouthEast(int const *params);
void ActionWalkNorthWest(int const *params);
void ActionWalkSouthWest(int const *params);
void ActionStopWalking(int const *params);
void ActionCenter(int const *params);
void ActionShapeBrowser(int const *params);
void ActionShapeBrowserHelp(int const *params);
void ActionCreateShape(int const *params);
void ActionDeleteObject(int const *params);
void ActionDeleteSelected(int const *params);
void ActionMoveSelected(int const *params);
void ActionToggleEggs(int const *params);
void ActionGodMode(int const *params);
void ActionGender(int const *params);
void ActionCheatHelp(int const *params);
void ActionMapeditHelp(int const *params);
void ActionInfravision(int const *params);
void ActionSkipLift(int const *params);
void ActionLevelup(int const *params);
void ActionMapEditor(int const *params);
void ActionHackMover(int const *params);
void ActionMapTeleport(int const *params);
void ActionWriteMiniMap(int const *params);
void ActionTeleport(int const *params);
void ActionNextMapTeleport(int const *params);
void ActionTime(int const *params);
void ActionWizard(int const *params);
void ActionHeal(int const *params);
void ActionCheatScreen(int const *params);
void ActionPickPocket(int const *params);
void ActionNPCNumbers(int const *params);
void ActionGrabActor(int const *params);

void ActionCut(int const *params);
void ActionCopy(int const *params);
void ActionPaste(int const *params);

void ActionPlayMusic(int const *params);
void ActionNaked(int const *params);
void ActionPetra(int const *params);
void ActionSkinColour(int const *params);
void ActionNotebook(int const *params);
void ActionSoundTester(int const *params);
void ActionTest(int const *params);

#endif
