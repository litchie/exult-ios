/*
 *  Copyright (C) 2001  The Exult Team
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

#include "exultmenu.h"
#include "mouse.h"
#include "gamewin.h"
#include "ibuf8.h"
#include "game.h"
#include "Audio.h"
#include "menulist.h"
#include "font.h"
#include "cheat.h"
#include "Configuration.h"
#include "txtscroll.h"

extern Configuration *config;
extern Cheat cheat;
extern bool get_play_intro(void);
extern void set_play_intro(bool);
extern bool get_play_1st_scene(void);
extern void set_play_1st_scene(bool);

ExultMenu::ExultMenu(Game_window *gw)
{
	gwin = gw;
	ibuf = gwin->get_win()->get_ib8();
	calc_win();
	fontManager.add_font("CREDITS_FONT", "<DATA>/exult.flx", 9, 1);
	exult_flx.load("<DATA>/exult.flx");
}

ExultMenu::~ExultMenu()
{
}

void ExultMenu::calc_win()
{
	topx = (gwin->get_width()-320)/2;
	topy = (gwin->get_height()-200)/2;
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
	menuy = topy+120;
}


void ExultMenu::setup()
{
	Font *font = fontManager.get_font("CREDITS_FONT");
	MenuList menu;
	
	MenuChoice *palfades = new MenuChoice(exult_flx.get_shape(0x16,1),
			      exult_flx.get_shape(0x16,0),
			      centerx, menuy-33, font);
	palfades->add_choice("Off");
	palfades->add_choice("On");
	palfades->set_choice(gwin->get_fades_enabled()?1:0);
	menu.add_entry(palfades);
	
	MenuChoice *midiconv = new MenuChoice(exult_flx.get_shape(0x14,1),
			      exult_flx.get_shape(0x14,0),
			      centerx, menuy-22, font);
	midiconv->add_choice("None");
	midiconv->add_choice("GM");
	midiconv->add_choice("GS");
	midiconv->add_choice("GS127");
	midiconv->add_choice("GS127DRUM");
	midiconv->set_choice(Audio::get_ptr()->get_midi()->get_music_conversion());
	menu.add_entry(midiconv);
	
	MenuChoice *sfxconv = new MenuChoice(exult_flx.get_shape(0x15,1),
			      exult_flx.get_shape(0x15,0),
			      centerx, menuy-11, font);
	sfxconv->add_choice("None");
	sfxconv->add_choice("GS");
	sfxconv->set_choice(Audio::get_ptr()->get_midi()->get_effects_conversion()==XMIDI_CONVERT_GS127_TO_GS?1:0);
	menu.add_entry(sfxconv);
	
	MenuChoice *playintro = new MenuChoice(exult_flx.get_shape(0x0B,1),
			      exult_flx.get_shape(0x0B,0),
			      centerx, menuy, font);
	playintro->add_choice("Off");
	playintro->add_choice("On");
	playintro->set_choice(get_play_intro()?1:0);
	menu.add_entry(playintro);
	
	MenuChoice *playscene = new MenuChoice(exult_flx.get_shape(0x12,1),
			      exult_flx.get_shape(0x12,0),
			      centerx, menuy+11, font);
	playscene->add_choice("Off");
	playscene->add_choice("On");
	playscene->set_choice(get_play_1st_scene()?1:0);
	menu.add_entry(playscene);

	MenuChoice *fullscreen = new MenuChoice(exult_flx.get_shape(0x0C,1),
			      exult_flx.get_shape(0x0C,0),
			      centerx, menuy+22, font);
	fullscreen->add_choice("Off");
	fullscreen->add_choice("On");
	fullscreen->set_choice(gwin->get_win()->is_fullscreen()?1:0);
	menu.add_entry(fullscreen);
	
	MenuChoice *cheating = new MenuChoice(exult_flx.get_shape(0x0D,1),
				      exult_flx.get_shape(0x0D,0),
				      centerx, menuy+33, font);
	cheating->add_choice("Off");
	cheating->add_choice("On");
	cheating->set_choice(cheat()?1:0);
	menu.add_entry(cheating);
	
	MenuEntry *ok = new MenuEntry(exult_flx.get_shape(0x0E,1),
		      exult_flx.get_shape(0x0E,0),
		      centerx-64, menuy+55);
	int ok_button = menu.add_entry(ok);
	
	MenuEntry *cancel = new MenuEntry(exult_flx.get_shape(0x0F,1),
			 exult_flx.get_shape(0x0F,0),
			 centerx+64, menuy+55);
	int cancel_button = menu.add_entry(cancel);
	
	menu.set_selection(0);
	gwin->clear_screen();
	for(;;) {
		pal.apply();
		int entry = menu.handle_events(gwin,menu_mouse);
		if(entry==ok_button) {
			pal.fade_out(c_fade_out_time);
			gwin->clear_screen();
			// Palette fades
			gwin->set_fades_enabled(palfades->get_choice()==1);
			config->set("config/video/disable_fades",gwin->get_fades_enabled()?"no":"yes",true);
			// Midi Conversion
			Audio::get_ptr()->get_midi()->set_music_conversion(midiconv->get_choice());
			// SFX Conversion
			Audio::get_ptr()->get_midi()->set_effects_conversion(sfxconv->get_choice()==1?XMIDI_CONVERT_GS127_TO_GS:XMIDI_CONVERT_NOCONVERSION);
			// Play Intro
			set_play_intro(playintro->get_choice()==1);
			// Play 1st scene
			set_play_1st_scene(playscene->get_choice()==1);
			// Full screen
			if(((fullscreen->get_choice()==0)&&(gwin->get_win()->is_fullscreen()))||
			   ((fullscreen->get_choice()==1)&&(!gwin->get_win()->is_fullscreen())))
				gwin->get_win()->toggle_fullscreen();
			config->set("config/video/fullscreen",gwin->get_win()->is_fullscreen()?"yes":"no",true);
			// Cheating
			cheat.set_enabled(cheating->get_choice()==1);
			calc_win();
			return;
		} else if (entry==cancel_button) {
			pal.fade_out(c_fade_out_time);
			gwin->clear_screen();
			return;
		}
	}
}

Exult_Game ExultMenu::run()
{
	ExultDataSource *midi_data = new ExultDataSource("<DATA>/exult.flx", 8);
	XMIDI midfile(midi_data, XMIDI_CONVERT_NOCONVERSION);
	Audio::get_ptr()->start_music(&midfile, true);
	
	ExultDataSource mouse_data("<DATA>/exult.flx", 16);
	menu_mouse = new Mouse(gwin, mouse_data);
	
	gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 0));
	pal.load("<DATA>/exult.flx",5);
	pal.fade_in(c_fade_in_time);
	wait_delay(2000);
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 0x06, 0x07, 0x0A, 0x01, 0x00 , 0x11 };
	int num_choices = sizeof(menuchoices)/sizeof(int);
		
	for(int i=0; i<num_choices; i++) {
		menu->add_entry(new MenuEntry(exult_flx.get_shape(menuchoices[i],1),
					      exult_flx.get_shape(menuchoices[i],0),
					      centerx, menuy+i*11+(i<2?0:11)));
	}
	menu->set_selection(0);
	Exult_Game sel_game = NONE;
	do {
		gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 1));
		switch(menu->handle_events(gwin, menu_mouse)) {
		case 5:
		case -1: // Exit
			pal.fade_out(c_fade_out_time);
			Audio::get_ptr()->stop_music();
			std::exit(0);
		case 0: // Black Gate
			pal.fade_out(c_fade_out_time);
			sel_game = BLACK_GATE;
			break;
		case 1: // Serpent Isle
			pal.fade_out(c_fade_out_time);
			sel_game = SERPENT_ISLE;
			break;
		case 2: // Setup
			pal.fade_out(c_fade_out_time);
			setup();
			pal.apply();
			break;
		case 3: // Exult Credits
			{
				pal.fade_out(c_fade_out_time);
				TextScroller credits("<DATA>/exult.flx", 0x03, 
						     fontManager.get_font("CREDITS_FONT"),
						     exult_flx.extract_shape(0x13));
				credits.run(gwin,pal);
				gwin->clear_screen();
				pal.apply();
			}
			break;
		case 4: // Exult Quotes
			{
				pal.fade_out(c_fade_out_time);
				TextScroller quotes("<DATA>/exult.flx", 0x02, 
						    fontManager.get_font("CREDITS_FONT"),
			     			    exult_flx.extract_shape(0x13));
				quotes.run(gwin,pal);
				gwin->clear_screen();
				pal.apply();
			}
			break;
		default:
			break;
		}
	} while(sel_game==NONE);
	delete menu;
	
	gwin->clear_screen();
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
	delete midi_data;
	return sel_game;
}
