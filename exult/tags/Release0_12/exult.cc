/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Exult.cc - X-windows Ultima7 map browser.
 **
 **	Written: 7/22/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#if !(defined(XWIN) || defined(DOS))
#define __WIN32
#endif

#include <stdlib.h>

#ifndef __WIN32

#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

#include "gamewin.h"
#include "fnames.h"
#include "SDL.h"
#include "SDL_syswm.h"

/*
 *	Globals:
 */
Game_window *gwin = 0;
unsigned char quitting_time = 0;	// Time to quit.

/*
 *	Local functions:
 */
static void Init();
static int Play();
static void Handle_keystroke(SDLKey ch, int shift);


/*
 *	A handy breakpoint.
 */

static void Breakpoint
	(
	)
	{
	return;
	}

/*
 *	Main program.
 */

int main
	(
	int argc,
	char *argv[]
	)
	{
	cout << "Exult V0." << RELNUM << 
				".  Copyright (C) 2000 J. S. Freedman\n";
	cout << "Text rendering done using the 'FreeType' font engine.\n";
	cout << "Low level graphics use the 'SDL' library.\n";
	Init();				// Create main window.
#if 0	/* Make this an option. */
	if (argc > 2)			// Specify chunkx, chunky on cmndline.
		gwin->set_chunk_offsets(atoi(argv[1]), atoi(argv[2]));
	else				// Else start in Trinsic.
#endif
		gwin->set_chunk_offsets(64, 136);
	struct stat sbuf;		// Create gamedat files 1st time.
	if (stat(U7NBUF_DAT, &sbuf) != 0 &&
	    stat(NPC_DAT, &sbuf) != 0)
		{
		cout << "Creating 'gamedat' files.\n";
		gwin->write_gamedat(INITGAME);
		}
	return (Play());
	}

static void Handle_event(SDL_Event& event);
#ifdef XWIN
static Display *display = 0;
#endif

/*
 *	Initialize and create main window.
 */
static void Init
	(
	)
	{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
		{
		cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
		exit(-1);
		}
	SDL_SysWMinfo info;		// Get system info.
	SDL_VERSION(&info.version);
#ifdef XWIN
	SDL_GetWMInfo(&info);
	display = info.info.x11.display;
	int screen_num = DefaultScreen(display);
	unsigned int display_width = DisplayWidth(display, screen_num);
	unsigned int display_height = DisplayHeight(display, screen_num);
					// Make window 1/2 size of screen.
	int w = display_width/2, h = display_height/2;
	if (w < 500)
		w = 500;
	if (h < 400)
		h = 400;
	gwin = new Game_window(w, h);
#else
	gwin = new Game_window(640, 480);
#endif
	}

/*
 *	Handle events until a flag is set.
 */

void Handle_events
	(
	unsigned char *stop
	)
	{
#ifdef XWIN
					// Get connection number.
	int xfd = ConnectionNumber(display);
#endif
	/*
	 *	Main event loop.
	 */
	while (!*stop)
		{
#ifdef XWIN
		fd_set rfds;		// Want a timer.
		struct timeval timer;
		timer.tv_sec = 0;
		timer.tv_usec = 20000;	// Try 1/50 second.
		FD_ZERO(&rfds);
		FD_SET(xfd, &rfds);
					// Wait for timeout or event.
		select(xfd + 1, &rfds, 0, 0, &timer);
#else					/* May use this for Linux too. */
		SDL_Delay(20);		// Try 1/50 second.
#endif
		SDL_Event event;
		while (!*stop && SDL_PollEvent(&event))
			Handle_event(event);
					// Get current time.
		unsigned long ticks = SDL_GetTicks();
					// Animate unless modal or dormant.
		if (gwin->have_focus() && gwin->get_mode() == 
							Game_window::normal)
			gwin->get_tqueue()->activate(ticks);
		gwin->show();		// Blit to screen if necessary.
		}
	}

/*
 *	Play game.
 */

static int Play()
	{
	Handle_events(&quitting_time);
	delete gwin;
	return (0);
	}

/*
 *	Handle an event.  This should work for all platforms.
 */

static void Handle_event
	(
	SDL_Event& event
	)
	{
					// For detecting double-clicks.
	static unsigned long last_b1_click = 0;
	unsigned int mask;
	char keybuf[10];
//cout << "Event " << (int) event.type << " received\n";
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
		gwin->end_intro();
		if (gwin->get_mode() != Game_window::normal)
			break;
					// Move sprite toward mouse
					//  when right button pressed.
		if (event.button.button == 3)
			gwin->start_actor(event.button.x, event.button.y);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 3)
			{
			if (gwin->get_mode() != Game_window::normal)
				break;
			gwin->stop_actor();
			}
		else if (event.button.button == 1)
			{
			if (gwin->get_mode() == Game_window::conversation)
				{	// In a conversation.
				gwin->conversation_choice(
					event.button.x, event.button.y);
				break;
				}
			unsigned long curtime = SDL_GetTicks();
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				gwin->double_clicked(event.button.x, 
							event.button.y);
				break;
				}
			last_b1_click = curtime;
					// Identify item(s) clicked on.
			gwin->show_items(event.button.x, event.button.y);
			}
		break;
	case SDL_MOUSEMOTION:		// Moving with right button down.
		if (gwin->get_mode() != Game_window::normal)
			break;
		if (event.motion.state != 0)
			gwin->start_actor(event.motion.x, event.motion.y);
		break;
	case SDL_ACTIVEEVENT:
		if (event.active.state & SDL_APPINPUTFOCUS)
			{
			if (event.active.gain)
				gwin->get_focus();
			else
				gwin->lose_focus();
			}
		if (event.active.state & SDL_APPACTIVE)
					// Became active.
			if (event.active.gain)
				gwin->init_actors();
		break;
#if 0
	case ConfigureNotify:		// Resize.
		gwin->resized(event.xconfigure.window,
			event.xconfigure.width, event.xconfigure.height);
		break;
#endif
	case SDL_QUIT:
		quitting_time = 1;
		break;
	case SDL_KEYDOWN:			// Keystroke.
		Handle_keystroke(event.key.keysym.sym,
			event.key.keysym.mod & KMOD_SHIFT);
		break;
		}
	}

/*
 *	Handle a keystroke.
 */

static void Handle_keystroke
	(
	SDLKey sym,
	int shift
	)
	{
	static int shape_cnt = 0x360, shape_frame = 0;
	static int face_cnt = -1, face_frame = 0;
	static int gump_cnt = 4, gump_frame = 0;
	gwin->end_intro();
	switch (sym)
		{
	case SDLK_PLUS:			// Brighten.
		gwin->brighten(20);
		break;
	case SDLK_MINUS:		// Darken.
		gwin->brighten(-20);
		break;
	case SDLK_b:
		Breakpoint();
		break;
	case SDLK_q:
	case SDLK_ESCAPE:			// ESC key.
		quitting_time = 1;
		break;
	case SDLK_l:			// Decrement skip_lift.
		if (gwin->skip_lift == 16)
			gwin->skip_lift = 11;
		else
			gwin->skip_lift--;
		if (gwin->skip_lift == 0)
			gwin->skip_lift = 16;
		cout << "Skip_lift = " << gwin->skip_lift << '\n';
					// FALL THROUGH.
	case SDLK_p:			// Rerender image.
		gwin->paint();
		break;
	case SDLK_s:		// Show next shape.
#if 0
		shape_frame = 0;
		if (++shape_cnt == gwin->get_num_shapes())
			shape_cnt = 0;
		cout << "Painting shape " << shape_cnt << '\n';
		gwin->paint();
		gwin->paint_shape(gwin->get_win(),
			200, 200, shape_cnt, shape_frame);
#else
		cout << "Num. gumps = " << gwin->get_num_gumps() << '\n';
		gump_frame = 0;
		if (++gump_cnt == gwin->get_num_gumps())
			gump_cnt = 0;
		cout << "Painting gump " << gump_cnt << '\n';
		gwin->paint();
		gwin->paint_gump(gwin->get_win(),
			200, 200, gump_cnt, gump_frame);
#endif
		break;
	case 'S':		// Show prev. shape.
		shape_frame = 0;
		if (--shape_cnt < 0)
			shape_cnt = gwin->get_num_shapes() - 1;
		cout << "Painting shape " << shape_cnt << '\n';
		gwin->paint();
		gwin->paint_shape(gwin->get_win(),
			200, 200, shape_cnt, shape_frame);
		break;
	case SDLK_f:			// Show next frame.
		cout << "Frame # " << ++shape_frame << '\n';
		gwin->paint();
#if 1
		gwin->paint_shape(gwin->get_win(),
			200, 200, shape_cnt, shape_frame);
#else
		gwin->paint_shape(gwin->get_win(),
			200, 200, face_cnt, face_frame);
#endif
		break;
	case SDLK_RIGHT:
		gwin->view_right();
		break;
	case SDLK_LEFT:
		gwin->view_left();
		break;
	case SDLK_DOWN:
		gwin->view_down();
		break;
	case SDLK_UP:
		gwin->view_up();
		break;
		}
	}


#if 0
// The old win32 code.
//#ifdef __WIN32

UINT g_nCmdShow;
HINSTANCE g_hInstance;

LONG APIENTRY Handle_event (HWND, UINT, UINT, LONG);

void gXXXXettimeofday(timeval* tv, int x); //in objs.cpp

UINT TimerID;

//main function
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow ) {
	OSVERSIONINFO osVer;
  osVer.dwOSVersionInfoSize = sizeof(osVer);
  if (!GetVersionEx(&osVer)) return false;
  if (osVer.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
    MessageBox(NULL, "This version of Exult requires Window 95 or higher.",
	"Exult", MB_OK );
    return false;
  }

  g_nCmdShow = nCmdShow;
  g_hInstance = hInstance;

  //TO DO: Parse commandline

  Init(); //init window class and create game window

  //init location (Trinsic)
  gwin->set_chunk_offsets(64, 135);

  return (Play());
}

int Play() {
	MSG msg;

	gwin->paint();			// Get backgrounds loaded.
	gwin->init_actors();		// Place actors in world.
	gwin->paint();
	gwin->show();

  HWND hWnd = gwin->get_win()->win;

  TimerID = SetTimer(hWnd, 1, 20, NULL);

  while (GetMessage(&msg, NULL, 0, 0)) { //message loop
	    TranslateMessage(&msg); //necessary?
	    DispatchMessage(&msg);
  }

  KillTimer(hWnd, 1);

  delete gwin;

  return msg.wParam; //end program
}

//create image window instance
HWND InitInstance (int cwidth, int cheight) {
  HWND hWnd;
  RECT ClientRect;
  int width, height;

  ClientRect.left = 50;
  ClientRect.top = 50;
  ClientRect.right = 50 + cwidth;
  ClientRect.bottom = 50 + cheight;

  AdjustWindowRect(&ClientRect, WS_OVERLAPPEDWINDOW, false);
  width = ClientRect.right - ClientRect.left;
  height = ClientRect.bottom - ClientRect.top;

  //TO DO: adjust to screen size

  hWnd = CreateWindow("ExultImgWClass", "Exult", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, g_hInstance, NULL);

  if (!hWnd) return NULL;

  ShowWindow(hWnd, g_nCmdShow);
//  UpdateWindow(hWnd);
  return hWnd;
}

BOOL ShiftDown ( ) {
  return (GetKeyState(VK_SHIFT) & 0x8000000);
}

BOOL CtrlDown () {
  return (GetKeyState(VK_CONTROL) & 0x8000000);
}

void TranslateCoord(HWND hWnd, int &xPos, int &yPos) {
  RECT ClientRect;
  GetClientRect(hWnd, &ClientRect);
  int cwidth = ClientRect.right;
  int cheight = ClientRect.bottom;

  if (cwidth*cheight) {
    xPos = xPos*320/cwidth;  // horizontal position of cursor
    yPos = yPos*200/cheight;  // vertical position of cursor
  }
}

//handle message
LONG APIENTRY Handle_event (HWND hWnd, UINT Message, UINT wParam, LONG lParam) {
  static BOOL doubleclicked = false;
  PAINTSTRUCT PaintInfo;

  int xPos = LOWORD(lParam);
  int yPos = HIWORD(lParam);

  UINT q;
  switch (Message) {
	case WM_DESTROY: //quit
      PostQuitMessage(0);
      break;
    case WM_LBUTTONDOWN: //left mouse pressed
		  gwin->end_intro();
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode

      if (!CtrlDown()) break;

    case WM_MBUTTONDOWN: //middle mouse or Ctrl-Left mouse (fall-through)
      TranslateCoord(hWnd, xPos, yPos);
			if (ShiftDown()) // Show abs. game loc. (for scripts).
				gwin->show_game_location(xPos, yPos);
			else
				gwin->debug_shape(hWnd, xPos, yPos);
      break;

    case WM_LBUTTONDBLCLK: //left mouse double clicked
      TranslateCoord(hWnd, xPos, yPos);
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode
			gwin->double_clicked(hWnd, xPos, yPos);
      doubleclicked = true;
      break;

    case WM_RBUTTONDOWN: //right mouse pressed
		  gwin->end_intro();
			gwin->stop_showing_item();
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode

      TranslateCoord(hWnd, xPos, yPos);

      gwin->start_actor(xPos, yPos);

      SetCapture(hWnd); //capture mouse while holding right button
      break;

    case WM_LBUTTONUP: //left mouse released
      if (doubleclicked) {
	doubleclicked = false;
	break;
      }
      TranslateCoord(hWnd, xPos, yPos);
			if (gwin->get_mode() == Game_window::conversation) {	// Conversation.
				gwin->converse(xPos, yPos);
	break;
      }
      gwin->show_items(xPos, yPos);
	break;

    case WM_RBUTTONUP: //right mouse released
      ReleaseCapture(); //release mouse capture
      gwin->stop_actor();
      break;

    case WM_MOUSEMOVE: //moving mouse
      if (wParam & MK_RBUTTON) //right mouse down?
	gwin->start_actor(xPos, yPos);
      break;

    case WM_TIMER: //timer event
		  struct timeval TVAL;
		  gXXXXettimeofday(&TVAL,0);
		gwin->get_tqueue()->activate(timer);
      gwin->show();
      break;

    case WM_KEYDOWN: //key pressed, ok?
      if (wParam >= 'A' && wParam <= 'Z' && !ShiftDown())
	wParam += 0x20;	 //convert to lowercase
      if ((char)wParam == 'q')
	PostQuitMessage(0);
      else {
/*	  char s[10];
	sprintf(s, "%x", (char)wParam);
	MessageBox(NULL,s,"Key pressed",MB_OK); */

	Handle_keystroke((char)wParam);
      }
      break;

    case WM_PAINT: //paint window
      if (gwin) {
	gwin->get_win(hWnd)->ScreenDC = BeginPaint(hWnd, &PaintInfo);
	gwin->show(hWnd);
	EndPaint(hWnd, &PaintInfo);
	gwin->get_win(hWnd)->ScreenDC = 0;
      }
      break;

    case WM_SIZING: case WM_SIZE:
      if (gwin)
	gwin->show(hWnd);
      break;

    default:
	return (DefWindowProc(hWnd, Message, wParam, lParam));
  }
  return 0;
}

void Init() {
  WNDCLASS wc;

  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)Handle_event;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "ExultImgWClass";

  if (!RegisterClass(&wc)) {
    MessageBox(NULL,"Error registering Window class","Exult",MB_OK);
    exit(1);
  }

  gwin = new Game_window(320, 200);
}

#endif //__WIN32
