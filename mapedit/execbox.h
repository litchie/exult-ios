/**
 **	Execbox.h - Execute a command-line program and display its output.
 **
 **	Written: 10/02/02 - JSF
 **/

#ifndef INCL_EXECBOX_H
#define INCL_EXECBOX_H	1

/*
Copyright (C) 2002 The Exult Team

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

#include <gtk/gtk.h>

/*
 *	This class can execute a command-line program and display its output.
 */
class Exec_box
	{
					// Pipes for talking to child:
	int child_stdin, child_stdout, child_stderr;
	int child_pid;			// Child's process ID.
	gint stdout_tag, stderr_tag;	// GDK tags for getting child's output.
	GtkText *box;			// Where we show responses.
	GtkStatusbar *status;		// For showing status.
public:
	Exec_box(GtkText *b, GtkStatusbar *s);
	~Exec_box();
	void kill_child();		// Kill process.
	bool check_child();		// Is child still running?
	void read_from_child(int id);	// Read and display.
					// Execute.
	bool exec(const char *file, char *argv[]);
	};

#endif
