/*
 *  Copyright (C) 2000-2001  The Exult Team
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
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "menulist.h"
#include "exultmenu.h"
#include "exult.h"
#include "mouse.h"
#include "gamewin.h"
#include "ibuf8.h"
#include "game.h"
#include "bggame.h"
#include "sigame.h"
#include "Audio.h"
#include "font.h"
#include "cheat.h"
#include "Configuration.h"
#include "txtscroll.h"
#include "data/exult_flx.h"

static bool get_play_intro(void);
static void set_play_intro(bool);
static bool get_play_1st_scene(void);
static void set_play_1st_scene(bool);

ExultMenu::ExultMenu(Game_window *gw)
{
	gwin = gw;
	ibuf = gwin->get_win()->get_ib8();
	calc_win();
	fontManager.add_font("CREDITS_FONT", "<DATA>/exult.flx", EXULT_FLX_FONT_SHP, 1);
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

	int menuypos = menuy-44;
	
	MenuChoice *scalemethod = new MenuChoice(exult_flx.get_shape(EXULT_FLX_SCALING_METHOD_SHP,1),
			      exult_flx.get_shape(EXULT_FLX_SCALING_METHOD_SHP,0),
			      centerx, menuypos, font);
	for (int i = 0; i < Image_window::NumScalers; i++)
		scalemethod->add_choice(Image_window::get_name_for_scaler(i));
	scalemethod->set_choice(gwin->get_win()->get_scaler());
	menu.add_entry(scalemethod);
	menuypos+=11;
	
	MenuChoice *palfades = new MenuChoice(exult_flx.get_shape(EXULT_FLX_PALETTE_FADES_SHP,1),
			      exult_flx.get_shape(EXULT_FLX_PALETTE_FADES_SHP,0),
			      centerx, menuypos, font);
	palfades->add_choice("Off");
	palfades->add_choice("On");
	palfades->set_choice(gwin->get_fades_enabled()?1:0);
	menu.add_entry(palfades);
	menuypos+=11;

	MenuChoice *midiconv = 0;
#ifdef ENABLE_MIDISFX
	MenuChoice *sfxconv = 0;
#endif


	if (Audio::get_ptr()->get_midi()) {
	  midiconv = new MenuChoice(exult_flx.get_shape(EXULT_FLX_MIDI_CONVERSION_SHP,1),
				  exult_flx.get_shape(EXULT_FLX_MIDI_CONVERSION_SHP,0),
				  centerx, menuypos, font);

	  midiconv->add_choice("None");
	  midiconv->add_choice("GM");
	  midiconv->add_choice("GS");
	  midiconv->add_choice("GS127");
	  midiconv->add_choice("MP3");

	  midiconv->set_choice(Audio::get_ptr()->get_midi()->get_music_conversion());
	  menu.add_entry(midiconv);
	  menuypos+=11;

#ifdef ENABLE_MIDISFX	
	  MenuChoice *sfxconv = new MenuChoice(exult_flx.get_shape(EXULT_FLX_SFX_CONVERSION_SHP,1),
					     exult_flx.get_shape(EXULT_FLX_SFX_CONVERSION_SHP,0),
					     centerx, menuypos, font);
	  sfxconv->add_choice("None");
	  sfxconv->add_choice("GS");
	  sfxconv->set_choice(Audio::get_ptr()->get_midi()->get_effects_conversion()==XMIDI_CONVERT_GS127_TO_GS?1:0);

	  menu.add_entry(sfxconv);
	  menuypos+=11;
#endif
	}
	
	MenuChoice *playintro = new MenuChoice(exult_flx.get_shape(EXULT_FLX_PLAY_INTRO_SHP,1),
			      exult_flx.get_shape(EXULT_FLX_PLAY_INTRO_SHP,0),
			      centerx, menuypos, font);
	playintro->add_choice("Off");
	playintro->add_choice("On");
	playintro->set_choice(get_play_intro()?1:0);
	menu.add_entry(playintro);
	menuypos+=11;

	
	MenuChoice *playscene = new MenuChoice(exult_flx.get_shape(EXULT_FLX_PLAY_1ST_SCENE_SHP,1),
			      exult_flx.get_shape(EXULT_FLX_PLAY_1ST_SCENE_SHP,0),
			      centerx, menuypos, font);
	playscene->add_choice("Off");
	playscene->add_choice("On");
	playscene->set_choice(get_play_1st_scene()?1:0);
	menu.add_entry(playscene);
	menuypos+=11;


	MenuChoice *fullscreen = new MenuChoice(exult_flx.get_shape(EXULT_FLX_FULL_SCREEN_SHP,1),
			      exult_flx.get_shape(EXULT_FLX_FULL_SCREEN_SHP,0),
			      centerx, menuypos, font);
	fullscreen->add_choice("Off");
	fullscreen->add_choice("On");
	fullscreen->set_choice(gwin->get_win()->is_fullscreen()?1:0);
	menu.add_entry(fullscreen);
	menuypos+=11;

	MenuChoice *cheating = new MenuChoice(exult_flx.get_shape(EXULT_FLX_CHEATING_SHP,1),
				      exult_flx.get_shape(EXULT_FLX_CHEATING_SHP,0),
				      centerx, menuypos, font);
	cheating->add_choice("Off");
	cheating->add_choice("On");
	cheating->set_choice(cheat()?1:0);
	menu.add_entry(cheating);
	menuypos+=11;

	MenuEntry *ok = new MenuEntry(exult_flx.get_shape(EXULT_FLX_OK_SHP,1),
		      exult_flx.get_shape(EXULT_FLX_OK_SHP,0),
		      centerx-64, menuy+55);
	int ok_button = menu.add_entry(ok);
	
	MenuEntry *cancel = new MenuEntry(exult_flx.get_shape(EXULT_FLX_CANCEL_SHP,1),
			 exult_flx.get_shape(EXULT_FLX_CANCEL_SHP,0),
			 centerx+64, menuy+55);
	int cancel_button = menu.add_entry(cancel);
	
	menu.set_selection(0);
	gwin->clear_screen(true);
	for(;;) {
		pal.apply();
		int entry = menu.handle_events(gwin,menu_mouse);
		if(entry==ok_button) {
			pal.fade_out(c_fade_out_time);
			gwin->clear_screen(true);
			// Scaling Method
			int scaler = scalemethod->get_choice();
			if(scaler!=gwin->get_win()->get_scaler()) {
				gwin->resized(
					gwin->get_win()->get_width(),
					gwin->get_win()->get_height(),
					gwin->get_win()->get_scale(),
					scalemethod->get_choice()
				);
				if (scaler > Image_window::NoScaler && scaler < Image_window::NumScalers)
					config->set("config/video/scale_method",Image_window::get_name_for_scaler(scaler),true);
			}
			// Palette fades
			gwin->set_fades_enabled(palfades->get_choice()==1);
			config->set("config/video/disable_fades",gwin->get_fades_enabled()?"no":"yes",true);

			if (Audio::get_ptr()->get_midi()) {
			  if (midiconv) {
			    // Midi Conversion
			    Audio::get_ptr()->get_midi()->set_music_conversion(midiconv->get_choice());
			  }
#ifdef ENABLE_MIDISFX
			  if (sfxconv) {
			  // SFX Conversion
			    Audio::get_ptr()->get_midi()->set_effects_conversion(sfxconv->get_choice()==1?XMIDI_CONVERT_GS127_TO_GS:XMIDI_CONVERT_NOCONVERSION);
			  }
#endif
			}
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
			gwin->clear_screen(true);
			return;
		}
	}
}

Exult_Game ExultMenu::run()
{
	Font *font = fontManager.get_font("CREDITS_FONT");
	// Check for the games in the designated directories.
	bool bg_installed = BG_Game::is_installed();
	bool si_installed = SI_Game::is_installed();

	if(!bg_installed && !si_installed) {
		pal.load("<DATA>/exult.flx",EXULT_FLX_EXULT0_PAL);
		font->center_text(gwin->get_win()->get_ib8(),
				  centerx, topy+20, "WARNING");
		font->center_text(gwin->get_win()->get_ib8(),
				  centerx, topy+40, "Could not find the data files for either");
		font->center_text(gwin->get_win()->get_ib8(),
				  centerx, topy+50, "\"The Black Gate\" or \"Serpent Isle\".");
		font->center_text(gwin->get_win()->get_ib8(),
				  centerx, topy+60, "Please edit the configuration file");
		font->center_text(gwin->get_win()->get_ib8(),
				  centerx, topy+70, "and restart Exult");
		pal.apply();
		while(!wait_delay(200))
			;	
		throw quit_exception(1);

	}
	ExultDataSource *midi_data = new ExultDataSource("<DATA>/exult.flx", EXULT_FLX_MEDITOWN_MID);
	XMIDI midfile(midi_data, XMIDI_CONVERT_NOCONVERSION);
	
	if(Audio::get_ptr()->get_midi()->get_music_conversion() != XMIDI_CONVERT_MP3)
		Audio::get_ptr()->start_music(midfile.GetEventList(0), true);
	
	ExultDataSource mouse_data("<DATA>/exult.flx", EXULT_FLX_POINTERS_SHP);
	menu_mouse = new Mouse(gwin, mouse_data);
	
	gwin->paint_shape(topx,topy,exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 0));
	pal.load("<DATA>/exult.flx",EXULT_FLX_EXULT0_PAL);
	pal.fade_in(c_fade_in_time);
	wait_delay(2000);
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 
		EXULT_FLX_BLACK_GATE_SHP,
		EXULT_FLX_SERPENT_ISLE_SHP,
		EXULT_FLX_SETUP_SHP,
		EXULT_FLX_EXULT_CREDITS_SHP,
		EXULT_FLX_EXULT_QUOTES_SHP,
		EXULT_FLX_EXIT_SHP 
	};
	int num_choices = sizeof(menuchoices)/sizeof(int);
	int *menuentries = new int[num_choices];
	int entries = 0;
	int sfx_bg_ypos = -1, sfx_si_ypos = -1;
	
	int ypos = menuy-24;
	for(int i=0; i<num_choices; i++) {
		if((i==0 && bg_installed)||(i==1 && si_installed)||i>1) {
			menu->add_entry(new MenuEntry(exult_flx.get_shape(menuchoices[i],1),
						      exult_flx.get_shape(menuchoices[i],0),
						      centerx, ypos));
			if(i==0)
				sfx_bg_ypos = ypos;
			if(i==1)
				sfx_si_ypos = ypos;
			ypos += exult_flx.get_shape(menuchoices[i],0)->get_height()+2;
			menuentries[entries++]=i;
		}
		if(i<2)
			ypos+=5;
	}
	menu->set_selection(0);
	Exult_Game sel_game = NONE;
	
	do {
		gwin->paint_shape(topx,topy,exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 1));
		font->draw_text(gwin->get_win()->get_ib8(), 
					topx+320-font->get_text_width(VERSION), topy+190, VERSION);
		if (sfx_bg_ypos >= 0)
			gwin->paint_shape(centerx-80,sfx_bg_ypos, exult_flx.get_shape(EXULT_FLX_SFX_ICON_SHP, Audio::get_ptr()->can_sfx("blackgate")?1:0));
		if (sfx_si_ypos >= 0)
			gwin->paint_shape(centerx-80,sfx_si_ypos,exult_flx.get_shape(EXULT_FLX_SFX_ICON_SHP, Audio::get_ptr()->can_sfx("serpentisle")?1:0));
		int choice = menu->handle_events(gwin, menu_mouse);
		switch(choice<0?choice:menuentries[choice]) {
		case 5:
		case -1: // Exit
			pal.fade_out(c_fade_out_time);
			Audio::get_ptr()->stop_music();
			throw quit_exception();
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
				TextScroller credits("<DATA>/exult.flx", EXULT_FLX_CREDITS_TXT, 
						     fontManager.get_font("CREDITS_FONT"),
						     exult_flx.extract_shape(EXULT_FLX_EXTRAS_SHP));
				credits.run(gwin,pal);
				gwin->clear_screen(true);
				pal.apply();
			}
			break;
		case 4: // Exult Quotes
			{
				pal.fade_out(c_fade_out_time);
				TextScroller quotes("<DATA>/exult.flx", EXULT_FLX_QUOTES_TXT, 
						    fontManager.get_font("CREDITS_FONT"),
			     			    exult_flx.extract_shape(EXULT_FLX_EXTRAS_SHP));
				quotes.run(gwin,pal);
				gwin->clear_screen(true);
				pal.apply();
			}
			break;
		default:
			break;
		}
	} while(sel_game==NONE);
	delete[] menuentries;
	delete menu;
	
	gwin->clear_screen(true);
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
	delete midi_data;
	return sel_game;
}

bool get_play_intro (void)
{
	std::string yn;
	config->value("config/gameplay/skip_splash", yn, "no");
	return(yn=="no");
}

void set_play_intro (bool play)
{
	config->set("config/gameplay/skip_splash", play?"no":"yes", true);
}

bool get_play_1st_scene (void)
{
	std::string yn;
	config->value("config/gameplay/skip_intro", yn, "no");
	return(yn=="no");
}

void set_play_1st_scene (bool play)
{
	config->set("config/gameplay/skip_intro", play?"no":"yes", true);
}
