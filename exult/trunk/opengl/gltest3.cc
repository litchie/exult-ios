/**
 **	An OpenGL exercise.  The goal is to display a scene of blocks with
 **	orthographic projection similar to some old RPG's.  This was started
 **	with code from one of NeHe's lessons.
 **
 **	Written: 4/11/02 - JSF
 **/

#include <stdlib.h>
#include <iostream.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "model.h"

SDL_Surface *surface = 0;		// Where we display.
const int screenw = 640, screenh = 480;

using namespace Exult3d;

/*
 *	Time to exit.
 */
void Quit
	(
	int code			// Exit code.
	)
	{
	SDL_Quit();
	exit(code);
	}

/*
 *	Window was resized.
 */
void Resized
	(
	int new_width, int new_height
	)
	{
					// Set viewing area to whole window.
	glViewport(0, 0, new_width, new_height);
	glMatrixMode(GL_PROJECTION);	// Set up orthogonal volume.
	glLoadIdentity();

	glOrtho(-16, 16, -16, 16, -16, 16);
	glMatrixMode(GL_MODELVIEW);	// Use model-view matrix from now on.
	glLoadIdentity();
	}

/*
 *	Handle keyboard.
 */
void Handle_key_press
	(
	SDL_keysym *keysym
	)
	{
	switch (keysym->sym)
		{
	case SDLK_ESCAPE:		// ESC quits.
		Quit(0);
		break;
	case SDLK_F1:			// Toggle fullscreen.
		SDL_WM_ToggleFullScreen(surface);
		break;
	default:
		break;
		}
	}

/*
 *	Initialize OpenGL.
 */
void InitGL
	(
	)
	{
	glShadeModel(GL_SMOOTH);	// Smooth shading.
	glClearColor(1, 1, 1, 0);	// Background is white.
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);	// Enable depth-testing.
	glDepthFunc(GL_LEQUAL);
//    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// ??

#if 1
					// Ambient light.
	static GLfloat ambient[] = {.5, .5, .5, 1.0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
					// A diffuse light source.
	static GLfloat diffuse[] = {1, 1, 1, 1.0};
	static GLfloat diffuse_pos[] = {0, 0, 8, 1};
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, diffuse_pos);
	glEnable(GL_LIGHT1);
#endif
	}

/*
 *	Draw the scene.
 */
void Render
	(
	Model3d& model			// Render this one model.
	)
	{
					// Clear screen & depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
					// Note that this is columnwise.

	glDisable(GL_LIGHTING);		// Disble lighting.
	glBegin(GL_LINES);		// Draw tile grid.
		{
		glColor3f(.5, .5, 1.0);	// Light blue for grid.
		for (int y = -15; y < 16; y++)	// Horizontal.
			{
		  	glVertex3f(-16, y, 0);
		  	glVertex3f(16,  y, 0);
			}
		for (int x = -15; x < 16; x++)	// Vertical.
			{
			glVertex3f(x, -16, 0);
			glVertex3f(x, 16,  0);
			}
		}
	glEnd();
	glEnable(GL_LIGHTING);		// Enable lighting.
	Vector3 center = model.find_center();
	glTranslatef(-center.x, -center.y + 6, -center.z - 20);
//	glTranslatef(-center.x, -center.y + 16, -center.z);
	glRotatef(45, 1, 0, 0);
	glScalef(.1, .1, .1);
//	glScalef(.5, .5, .5);
	model.render();			// Show model.
	SDL_GL_SwapBuffers();		// Blit.
	}

/*
 *	Main routine.
 */
int main
	(
	int argc, 
	char **argv
	)
	{
	bool done = false;		// Set when time to quit.
	bool active = true;		// Is window active.
	if (argc < 2)
		{
		cerr << "Provide a .3ds model file" << endl;
		Quit(1);
		}
	const char *model_name = argv[1];
					// Init. SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
		cerr << "SDL_Init() failed: " << SDL_GetError() << endl;
		Quit(1);
		}
					// Get info. about video.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
	if (!vinfo)
		{
		cerr << "SDL_GetVideoInfo() failed: " << SDL_GetError()
							<< endl;
		Quit(1);
		}
					// Set up SDL video flags.
	int video_flags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER |
			SDL_HWPALETTE | SDL_RESIZABLE;
					// Can surface be in video RAM?
	if (vinfo->hw_available)
		video_flags |= SDL_HWSURFACE;
	else
		video_flags |= SDL_SWSURFACE;
	if (vinfo->blit_hw)		// Hardware blits?
		video_flags |= SDL_HWACCEL;
					// Want double-buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
					// Allocate surface: 640x480x16bpp.
	surface = SDL_SetVideoMode(screenw, screenh, 16, video_flags);
	if (!surface)
		{
		cerr << "Couldn't allocate surface: " << SDL_GetError() <<
								endl;
		Quit(1);
		}
	InitGL();			// Initialize OpenGL.
					// ++++++TESTING
	Model3d model;
	model.import3ds(model_name);
	Resized(screenw, screenh);	// Set size.
					// Main event loop.
	while (!done)
		{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		switch (event.type)
			{
		case SDL_ACTIVEEVENT:	// Lost/gained focus.
			active = (event.active.gain != 0);
			break;
		case SDL_VIDEORESIZE:	// Window resized.
			surface = SDL_SetVideoMode(event.resize.w,
					event.resize.h, 16, video_flags);
			if (!surface)
				{
				cerr << "Resize failed: " << SDL_GetError() <<
								endl;
				Quit(1);
				}
			Resized(event.resize.w, event.resize.h);
			break;
		case SDL_KEYDOWN:
			Handle_key_press(&event.key.keysym);
			break;
		case SDL_QUIT:
			done = true;
			}
		if (active)
			Render(model);	// Paint it all.
		}
	Quit(0);
	return 0;
	}
