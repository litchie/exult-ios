/*
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

#include "files/U7file.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "game.h"
#include "palette.h"
#include "databuf.h"
#include "data/credits.h"
#include "data/quotes.h"

Game *game = 0;
static Exult_Game game_type = BLACK_GATE;

static char av_name[17] = "";
static int av_sex = -1;
static int av_skin = -1;

Game::Game() : menushapes(MAINSHP_FLX)
{
	gwin = Game_window::get_game_window();
	win = gwin->get_win();
	topx = (gwin->get_width()-320)/2;
	topy = (gwin->get_height()-200)/2;
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
	jive = false;

	if (!gwin->setup_mainshp_fonts())
			gwin->abort ("Unable to setup fonts from 'mainshp.flx' file.");
}

Game::~Game()
{
}

Game *Game::get_game()
{
	return game;
}
Exult_Game Game::get_game_type()
{
	return game_type;
}

Game *Game::create_game(const char *static_identity)
{

	if((!strcmp(static_identity,"ULTIMA7"))||(!strcmp(static_identity,"FORGE")))
                game_type = BLACK_GATE;
        else if((!strcmp(static_identity,"SERPENT ISLE"))||(!strcmp(static_identity,"SILVER SEED")))
                game_type = SERPENT_ISLE;

	switch(game_type) {
	case BLACK_GATE:
		game = new BG_Game();
		break;
	case SERPENT_ISLE:
		game = new SI_Game();
		break;
	default:
		game = 0;
	}
	return game;
}

bool Game::wait_delay(int ms)
{
	SDL_Event event;
	if(SDL_PollEvent(&event)) {
		if((event.type==SDL_KEYDOWN)||(event.type==SDL_MOUSEBUTTONDOWN))
			return true;
	}
	SDL_Delay(ms);
	return false;
}

void Game::clear_screen()
{
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);
}

void Game::play_flic(const char *archive, int index) 
{
	char *fli_buf;
	size_t len;
	U7object flic(archive, index);
	flic.retrieve(&fli_buf, len);
	playfli fli(fli_buf);
	fli.play(win);
	delete [] fli_buf;
}

void Game::play_audio(const char *archive, int index) 
{
	U7object speech(archive, index);
	// FIXME: should use a DataBuffer
	speech.retrieve("speech.voc");
	audio->playfile("speech.voc", false);
}

void Game::play_midi(int track,bool repeat)
{
	if (game_type == BLACK_GATE) audio->start_music(track,repeat,1);
	else if (game_type == SERPENT_ISLE) audio->start_music(track,repeat,2);
}

void Game::refresh_screen ()
{
	clear_screen();
}

void Game::add_shape(const char *name, int shapenum) 
{
	shapes[name] = shapenum;
}

int Game::get_shape(const char *name)
{
	return shapes[name];
}

void Game::add_resource(const char *name, const char *str, int num) 
{
	resources[name].str = (char *)str;
	resources[name].num = num;
}

str_int_pair Game::get_resource(const char *name)
{
	return resources[name];
}

int Game::show_text_line(int left, int right, int y, const char *s)
{
	//The texts used in the main menu contains backslashed sequences that
	//indicates the output format of the lines:
	// \Px   include picture number x (frame of MAINSHP.FLX shape 14h)
	// \C    center line
	// \L    left aligned to right center line
	// \R	 right aligned to left center line
	// |	 carriage return (stay on same line)
	// #xxx	 display character with number xxx

	char *txt = new char[strlen(s)+1];
	char *ptr = (char *)s;
	char *txtptr = txt;
	int ypos = y;
	int vspace = 2; // 2 extra pixels between lines
	// Align text to the left by default
	int align = -1;
	int xpos = left;
	int center = (right+left)/2;
	bool add_line = true;
	
	while(*ptr) {
		if(!strncmp(ptr,"\\P",2)) {
			int pix = *(ptr+2)-'0';
			ptr +=3;
			Shape_frame *shape = menushapes.get_shape(0x14,pix);
			gwin->paint_shape(center-shape->get_width()/2,
					  ypos, shape);
			ypos += shape->get_height()+vspace;
		} else if(!strncmp(ptr,"\\C",2)) {
			ptr += 2;
			align = 0;
		} else if(!strncmp(ptr,"\\L",2)) {
			ptr += 2;
			align = 1;
		} else if(!strncmp(ptr,"\\R",2)) {
			ptr += 2;
			align = -1;
		} else if(*ptr=='|' || *(ptr+1)==0) {
			if(*(ptr+1)==0 && *ptr!='|')
			{
				*txtptr++ = *ptr;
				add_line = false;
			}
			*txtptr = 0;
			
			if(align<0)
				xpos = center-gwin->get_text_width(MAINSHP_FONT1, txt);
			else if(align==0)
				xpos = center-gwin->get_text_width(MAINSHP_FONT1, txt)/2;
			else
				xpos = center;
			gwin->paint_text(MAINSHP_FONT1,txt,xpos,ypos);
			if(*ptr!='|') ypos += gwin->get_text_height(MAINSHP_FONT1)+vspace;
			txtptr = txt;	// Go to beginning of string
			++ptr;
		} else if(*ptr=='#') {
			ptr++;
			char numerical[4] = {0,0,0,0};
			char *num = numerical;
			while (*ptr >= '0' && *ptr <= '9')
				*num++ = *ptr++;
			*txtptr++ = atoi(numerical);
		} else
			*txtptr++ = *ptr++;
	}
	
	delete [] txt;
	if(add_line)
		ypos += gwin->get_text_height(MAINSHP_FONT1);
	return ypos;
}

vector<char *> *Game::load_text(const char *archive, int index)
{
	U7object txtobj(archive, index);
	size_t len;
		
	char *txt, *ptr, *end;
	txtobj.retrieve(&txt, len);
	ptr = txt;
	end = ptr+len;

	vector<char *> *text = new vector<char *>();
	while(ptr<end) {
		char *start = ptr;
		ptr = strchr(ptr, '\n');
		if(ptr) {
			if(*(ptr-1)=='\r') // It's CR/LF
				*(ptr-1) = 0;
			else
				*ptr = 0;
			text->push_back(strdup(start));
			ptr += 1;
		} else
			break;
	}
	delete [] txt;
	return text;
}

void Game::destroy_text(vector<char *> *text)
{
	for(int i=0; i<text->size(); i++)
		delete [] (*text)[i];
	delete text;
}

int Game::center_text(int font, const char *s, int x, int y)
{
	gwin->paint_text(font, s, x-gwin->get_text_width(font, s)/2, y);
	return y+gwin->get_text_height(font);
}

void Game::scroll_text(vector<char *> *text)
{
	int endy = topy+200;
	int starty = endy;
	int startline = 0;
	unsigned int maxlines = text->size();
	bool looping = true;
	int next_time = SDL_GetTicks() + 200;
	while(looping) {
		int ypos = starty;
		int curline = startline;
		clear_screen();
		do {
			if(curline==maxlines)
				break;
			ypos = show_text_line(topx, topx+320, ypos, (*text)[curline++]);
			if(ypos<topy) {		// If this line doesn't appear, don't show it next time
				++startline;
				starty = ypos;
				if(startline>=maxlines) {
					looping = false;
					break;
				}
			}
		} while (ypos<endy);
		pal.apply();
		while (next_time > SDL_GetTicks());
		next_time += 120;
		looping = looping && !wait_delay(0);
		if(!looping)
			pal.fade_out(30);
		starty --;
	}
}

void Game::show_menu()
	{
		int menuy = topy+110;

		top_menu();
		
		int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12, -1, -2 };
		int num_choices = sizeof(menuchoices)/sizeof(int);
		int selected = 2;
		SDL_Event event;
		char npc_name[16];
		sprintf(npc_name, "Exult");
		do {
			bool exit_loop = false;
			do {
				for(int i=0; i<num_choices; i++) {
					if(menuchoices[i]==-1) {
						center_text(MAINSHP_FONT1, "EXULT CREDITS", centerx, menuy+i*10);
					} else if(menuchoices[i]==-2) {
						center_text(MAINSHP_FONT1, "EXULT QUOTES", centerx, menuy+i*10);
					} else {
						Shape_frame *shape = menushapes.get_shape(menuchoices[i],i==selected);
						gwin->paint_shape(centerx-shape->get_width()/2,menuy+i*10,shape);
					}
				}		
				win->show();
				SDL_WaitEvent(&event);
				if(event.type==SDL_KEYDOWN) {
					switch(event.key.keysym.sym) {
					case SDLK_ESCAPE:
						pal.fade_out(30);
						exit(0);
						break;
					case SDLK_UP:
						--selected;
						if(selected<0)
							selected = num_choices-1;
						continue;
					case SDLK_DOWN:
						++selected;
						if(selected==num_choices)
							selected = 0;
						continue;
					case SDLK_RETURN:
						exit_loop = true;
						break;
					default:
						break;
					}
				}
			} while(!exit_loop);
			bool created = false;
			switch(selected) {
			case 0: // Intro
				pal.fade_out(30);
				play_intro();
				top_menu();
				break;
			case 2: // Journey Onwards
				created = gwin->init_gamedat(false);
				if(!created)
					break;
				// else fall through
			case 1: // New Game
				if(!created) {
					if(new_game(menushapes))
						selected = 2;
				} else
					selected = 2; // This will start the game
				break;
			case 3: // Credits
				pal.fade_out(30);
				show_credits();
				top_menu();
				break;
			case 4: // Quotes
				pal.fade_out(30);
				show_quotes();
				top_menu();
				break;
			case 5: // End Game
				pal.fade_out(30);
				end_game(true);
				top_menu();
				break;
			case 6: // Exult Credits
				pal.fade_out(30);
				show_exult_credits();
				top_menu();
				break;
			case 7: // Exult Quotes
				pal.fade_out(30);
				show_exult_quotes();
				top_menu();
				break;
			default:
				break;
			}
		} while(selected!=2);
		pal.fade_out(30);
		
		clear_screen();
		audio->stop_music();
	}
	
	
const char *Game::get_avname ()
{
	if (av_name[0])
		return av_name;
	else
		return NULL;
}

int Game::get_avsex ()
{
	return av_sex;
}
int Game::get_avskin ()
{
	return av_skin;
}

// Assume safe
void Game::set_avname (char *name)
{
	strcpy (av_name, name);
}

void Game::set_avsex (int sex)
{
	av_sex = sex;
}

void Game::set_avskin (int skin)
{
	av_skin = skin;
}

void Game::clear_avname ()
{
	av_name[0] = 0;
}

void Game::clear_avsex ()
{
	av_sex = -1;
}

void Game::clear_avskin ()
{
	av_skin = -1;
}

void Game::show_exult_credits()
	{
		
		vector<char *> *text = get_exult_credits();
		scroll_text(text);
		delete text; // Different way, because text is static
	}

void Game::show_exult_quotes()
	{
		
		vector<char *> *text = get_exult_quotes();
		scroll_text(text);
		delete text; // Different way, because text is static
	}

