/**
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#else

#include <windows.h>
//#include <windowsx.h>
#include <stdio.h>  //remove

#endif

#include "gamewin.h"
#include "fnames.h"

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
static void Handle_keystroke(int chr);

/*
 *	A handy breakpoint.
 */

static void Breakpoint
	(
	)
	{
	return;
	}

#ifndef __WIN32	  //Windows needs a WinMain, not main

/*
 *	Main program.
 */

int main
	(
	int argc,
	char *argv[]
	)
	{
	cout << "Exult V0.10.  Copyright (C) 2000 J. S. Freedman\n";
	cout << "Text rendering done using the 'FreeType' font engine.\n";
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

#endif	//!__WIN32

#ifdef XWIN

#include <X11/Xutil.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

Atom wm_delete_window, wm_protocols;
static void Handle_event(Display *);

/*
 *	Initialize and create main window.
 */
static void Init
	(
	)
	{
					// Open display.
	Display *display = XOpenDisplay(NULL);
	if (!display)
		{
		cerr << "Can't open display.\n";
		exit(-1);
		}
	Image_window::set_display(display);
					// Get screen info.
	int screen_num = DefaultScreen(display);
	unsigned int display_width = DisplayWidth(display, screen_num);
	unsigned int display_height = DisplayHeight(display, screen_num);
					// Make window 1/2 size of screen.
	gwin = new Game_window(display_width/2, display_height/2);
					// Indicate events we want.
	gwin->get_win()->select_input(ExposureMask | KeyPressMask |
		PointerMotionHintMask |
		ButtonPressMask | ButtonReleaseMask | Button3MotionMask | 
		FocusChangeMask | StructureNotifyMask);
					// Want to know if win. is closed.
	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, gwin->get_win()->get_win(),
							&wm_delete_window, 1);
	wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
	}

/*
 *	Handle events until a flag is set.
 */

void Handle_events
	(
	unsigned char *stop
	)
	{
	Display *display = Image_window::get_display();
					// Get connection number.
	int xfd = ConnectionNumber(display);
	/*
	 *	Main event loop.
	 */
	while (!*stop)
		{
		fd_set rfds;		// Want a timer.
		struct timeval timer;
		timer.tv_sec = 0;
		timer.tv_usec = 20000;	// Try 1/50 second.
		FD_ZERO(&rfds);
		FD_SET(xfd, &rfds);
					// Wait for timeout or event.
		select(xfd + 1, &rfds, 0, 0, &timer);
		while (!*stop && XPending(display) > 0)
			Handle_event(display);
					// Get current time.
		gettimeofday(&timer, 0);
					// Animate unless modal or dormant.
		if (gwin->have_focus() && gwin->get_mode() == 
							Game_window::normal)
			gwin->get_tqueue()->activate(timer);
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
	Display *display = Image_window::get_display();
	XCloseDisplay(display);		// All done.
	return (0);
	}

/*
 *	Handle an event.
 */

static void Handle_event
	(
	Display *display
	)
	{
	static Time last_b1_click = 0;	// For detecting double-clicks.
	XEvent event;
	Window root, win;		// Get window ID's.
	int rootx, rooty, winx, winy;	// Get positions.
	unsigned int mask;
	char keybuf[10];
	KeySym keysym;
	XComposeStatus compose;
	XNextEvent(display, &event);
	switch (event.type)
		{
	case ButtonPress:
		gwin->end_intro();
		if (gwin->get_mode() != Game_window::normal)
			break;
					// Move sprite toward mouse
					//  when right button pressed.
		if (event.xbutton.button == 3)
			gwin->start_actor(event.xbutton.x, event.xbutton.y);
#if 1
		else if (event.xbutton.button == 2)
			if (event.xbutton.state & ShiftMask)
					// Show abs. game loc. (for scripts).
				gwin->show_game_location(
					event.xbutton.x, event.xbutton.y);
			else
				gwin->debug_shape(event.xbutton.window,
					event.xbutton.x, event.xbutton.y);
#endif
		break;
	case ButtonRelease:
		if (event.xbutton.button == 3)
			gwin->stop_actor();
		else if (event.xbutton.button == 1)
			{
			if (gwin->get_mode() == Game_window::conversation)
				{	// In a conversation.
				gwin->conversation_choice(
					event.xbutton.x, event.xbutton.y);
				break;
				}
					// Last click within .5 secs?
			if (event.xbutton.time - last_b1_click < 500)
				{
				gwin->double_clicked(event.xbutton.window,
					event.xbutton.x, event.xbutton.y);
				break;
				}
			last_b1_click = event.xbutton.time;
					// Identify item(s) clicked on.
			gwin->show_items(event.xbutton.x, event.xbutton.y);
			}
		break;
	case MotionNotify:		// Moving with right button down.
		while (XCheckMaskEvent(display, Button3MotionMask, &event))
			;
		if (XQueryPointer(display, event.xmotion.window, &root,
				&win, &rootx, &rooty, &winx, &winy, &mask))
			gwin->start_actor(winx, winy);
		break;
	case Expose:			// Just repaint on last expose.
		if (event.xexpose.count == 0)
			gwin->show(event.xexpose.window);
		break;
	case FocusIn:
		gwin->get_focus(event.xexpose.window);
		break;
	case FocusOut:
		gwin->lose_focus(event.xexpose.window);
		break;
	case ConfigureNotify:		// Resize.
		gwin->resized(event.xconfigure.window,
			event.xconfigure.width, event.xconfigure.height);
		break;
	case ClientMessage:
		cout << "ClientMessage\n";
		if (event.xclient.format == 32 &&
			event.xclient.message_type == wm_protocols &&
			event.xclient.data.l[0] == (long) wm_delete_window)
				quitting_time = 1;
		break;
	case DestroyNotify:
		cout << "DestroyNotify\n";
		break;
	case MapNotify:
		gwin->init_actors();	// Place actors in world.
		cout << "MapNotify\n";
		break;
	case UnmapNotify:
		cout << "UnmapNotify\n";
		break;
	case KeyPress:			// Keystroke.
		XLookupString((XKeyEvent *) &event, 
			keybuf, sizeof(keybuf), &keysym, &compose);
		Handle_keystroke(keysym);
		break;
		}
	}

#endif	/* XWIN */

#ifdef DOS

#include <pc.h>
#include <keys.h>

/*
 *	Define X-names of keys:
 */
#define XK_plus		K_Plus
#define XK_minus	K_Dash
#define XK_b		'b'
#define XK_f		'f'
#define XK_F		'F'
#define XK_g		'g'
#define XK_G		'G'
#define XK_q		'q'
#define XK_l		'l'
#define XK_p		'p'
#define XK_s		's'
#define XK_S		'S'
#define XK_Right	K_Right
#define XK_Left		K_Left
#define XK_Down		K_Down
#define XK_Up		K_Up
#define XK_Escape	K_Escape

/*
 *	Initialize and create main window.
 */
static void Init
	(
	)
	{
	Image_window::set_display();	// Put screen in 320x200 mode.
	if (!Image_window::init_mouse())
		{
		Image_window::restore_display();
		cerr << "Can't initialize mouse.\n";
		exit(1);
		}
//	Image_window::set_mouse(20, 20);
	Image_window::show_mouse(1);
	gwin = new Game_window(320, 200);
	}

/*
 *	Play game.
 */

static int Play()
	{
	gwin->paint();			// Get backgrounds loaded.
	gwin->init_actors();		// Place actors in world.
	gwin->paint();
	gwin->show();
	/*
	 *	Main event loop.
	 */
	while (!quitting_time)
		{
					// Point actor is moving towards:
		static int actor_x = -1, actor_y = -1;
					// Prev. mouse button:
		static int prev_mouse = 0;
		struct timeval timer;
		if (kbhit())
			Handle_keystroke(getkey());
		int mouse = Image_window::get_mouse_buttons();
		if (mouse == 1)		// +++++Want to test for mouse-up.
			{
			if (prev_mouse != 1)
				{
				gwin->end_intro();
				int x, y;
				Image_window::get_mouse(x, y);
					// Identify item(s) clicked on.
				gwin->show_items(x, y);
				}
			}
		else if (mouse == 2)	// Move main actor.
			{
			gwin->end_intro();
			int x, y;
			Image_window::get_mouse(x, y);
			if (x != actor_x || y != actor_y)
				{
				actor_x = x; actor_y = y;
				gwin->start_actor(x, y);
				}
			}
		else if (mouse == 0)	// Buttons up.
			{
			if (actor_x != -1)
				{	// Stop actor.
				actor_x = -1;
				gwin->stop_actor();
				}
			}
		prev_mouse = mouse;
					// Get current time.
		gettimeofday(&timer, 0);
		gwin->get_tqueue()->activate(timer);
#if 0
		gwin->animate(timer);
#endif
		gwin->show();		// Update screen if necessary.
		}
	delete gwin;
	Image_window::restore_display();// Restore screen.
	return (0);
	}

#endif	/* DOS	*/

#ifdef __WIN32

#define XK_plus		0x6B
#define XK_minus	0x6D
#define XK_b		'b'
#define XK_f		'f'
#define XK_F		'F'
#define XK_q		'q'
#define XK_l		'l'
#define XK_p		'p'
#define XK_s		's'
#define XK_S		'S'
#define XK_Right	0x27
#define XK_Left		0x25
#define XK_Down		0x28
#define XK_Up		  0x26
#define XK_Escape	0x1B

#endif


/*
 *	Handle a keystroke.
 */

static void Handle_keystroke
	(
	int ch
	)
	{
	static int shape_cnt = 0x310, shape_frame = 0;
	static int face_cnt = -1, face_frame = 0;
	static int gump_cnt = -1, gump_frame = 0;
	gwin->end_intro();
	switch (ch)
		{
	case XK_plus:			// Brighten.
		gwin->brighten(20);
		break;
	case XK_minus:			// Darken.
	gwin->brighten(-20);
		break;
	case XK_b:
		Breakpoint();
		break;
	case XK_q:
	case XK_Escape:			// ESC key.
		quitting_time = 1;
		break;
	case XK_l:			// Decrement skip_lift.
		if (gwin->skip_lift == 16)
			gwin->skip_lift = 11;
		else
			gwin->skip_lift--;
		if (gwin->skip_lift == 0)
			gwin->skip_lift = 16;
		cout << "Skip_lift = " << gwin->skip_lift << '\n';
					// FALL THROUGH.
	case XK_p:			// Rerender image.
		gwin->paint();
		break;
	case XK_s:		// Show next shape.
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
	case XK_S:		// Show prev. shape.
		shape_frame = 0;
		if (--shape_cnt < 0)
			shape_cnt = gwin->get_num_shapes() - 1;
		cout << "Painting shape " << shape_cnt << '\n';
		gwin->paint();
		gwin->paint_shape(gwin->get_win(),
			200, 200, shape_cnt, shape_frame);
		break;
	case XK_f:			// Show next frame.
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
	case XK_Right:
		gwin->view_right();
		break;
	case XK_Left:
		gwin->view_left();
		break;
	case XK_Down:
		gwin->view_down();
		break;
	case XK_Up:
		gwin->view_up();
		break;
		}
	}

#ifdef __WIN32

UINT g_nCmdShow;
HINSTANCE g_hInstance;

LONG APIENTRY Handle_event (HWND, UINT, UINT, LONG);

void gettimeofday(timeval* tv, int x); //in objs.cpp

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
		  gettimeofday(&TVAL,0);
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
