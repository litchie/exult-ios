/**
 **	Execbox.cc - Execute a command-line program and display its output.
 **
 **	Written: 10/02/02 - JSF
 **/

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

#include "execbox.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <iostream.h>	/* Debugging only */

/*
 *	Create.
 */

Exec_box::Exec_box
	(
	GtkText *b,
	GtkStatusbar *s
	) : box(b), status(s),
	    child_stdin(-1), child_stdout(-1), child_stderr(-1),
	    child_pid(-1), stdout_tag(-1), stderr_tag(-1)
	{
	}

/*
 *	Close.
 */

Exec_box::~Exec_box
	(
	)
	{
	kill_child();
	}

/*
 *	End process and close pipes.
 */

void Exec_box::kill_child
	(
	)
	{
	if (child_pid > 0)
		kill(child_pid, SIGINT);
	if (child_stdin >= 0)
		close(child_stdin);
	if (child_stdout >= 0)
		close(child_stdout);
	if (child_stderr >= 0)
		close(child_stderr);
	if (stdout_tag >= 0)
		gdk_input_remove(stdout_tag);
	if (stderr_tag >= 0)
		gdk_input_remove(stderr_tag);
	child_pid = child_stdin = child_stdout = child_stderr = 
	stdout_tag = stderr_tag = -1;
	}

/*
 *	Close a pipe.
 */

inline void Close_pipe
	(
	int p[2]
	)
	{
	if (p[0] >= 0) close(p[0]);
	if (p[1] >= 0) close(p[1]);
	}

/*
 *	Close 3 sets of pipes.
 */

static void Close_pipes
	(
	int p0[2], int p1[2], int p2[2]
	)
	{
	Close_pipe(p0);
	Close_pipe(p1);
	Close_pipe(p2);
	}

/*
 *	See if child is still alive.
 *
 *	Output:	True if child is still running.
 */

bool Exec_box::check_child
	(
	)
	{
	if (child_pid < 0)
		return false;		// No child.
	int status;
					// Don't wait.
	int ret = waitpid(child_pid, &status, WNOHANG);
	if (ret != child_pid)
		return true;		// Still running.
	else
		{
		cout << "Exec_box:  Child done." << endl;
		//+++++++++Set status bar.
		return false;
		}
	}

/*
 *	Read from child & display in the text box.
 */

static void Read_from_child
	(
	gpointer data,			// ->Exex_box
	gint id,			// Pipe ID.
	GdkInputCondition condition
	)
	{
	Exec_box *box = (Exec_box *) data;
	box->read_from_child(id);
	}
void Exec_box::read_from_child
	(
	int id				// Pipe to read from.
	)
	{
	char buf[1024];
	int len;
	gtk_text_freeze(box);		// Looks better this way.
	while ((len = read(id, buf, sizeof(buf))) > 0)
		gtk_text_insert(box, NULL, NULL, NULL, buf, len);

	gtk_text_thaw(box);
	if (!check_child())		// Child done?
		kill_child();		// Clean up.
	}

/*
 *	Execute a new process.
 *
 *	Output:	False if failed.
 */

bool Exec_box::exec
	(
	const char *file, 		// PATH will be searched.
	char *argv[]			// Args.  1st is filename, last is 0.
	)
	{
					// Pipes for talking to child:
	int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
	stdin_pipe[0] = stdin_pipe[1] = stdout_pipe[0] = stdout_pipe[1] = 
	stderr_pipe[0] = stderr_pipe[1] = -1;
	kill_child();			// Kill running process.
	gtk_text_set_point(box, 0);	// Clear out old text.
	gtk_text_forward_delete(box, gtk_text_get_length(box));
					// Create pipes.
	if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0 ||
	    pipe(stderr_pipe) != 0)
		{			// Error.
		Close_pipes(stdin_pipe, stdout_pipe, stderr_pipe);
		return false;
		}
	child_pid = fork();		// Split in two.
	if (child_pid == -1)		// Failed?
		{
		Close_pipes(stdin_pipe, stdout_pipe, stderr_pipe);
		return false;
		}
	if (child_pid == 0)		// Are we the child?
		{
		close(0);		// Want to read from the pipe.
		dup(stdin_pipe[0]);
		Close_pipe(stdin_pipe);	// Done with these.
		close(1);		// Write to stdout through pipe.
		dup(stdout_pipe[1]);
		Close_pipe(stdout_pipe);
		close(2);		// Write to stderr through pipe.
		dup(stderr_pipe[1]);
		Close_pipe(stderr_pipe);
		execvp(file, argv);	// Become the new command.
		exit(-1);		// Gets here if exec failed.
		}
					// HERE, we're the parent.
	child_stdin = stdin_pipe[1];	// Writing here goes to child stdin.
	close(stdin_pipe[0]);
	child_stdout = stdout_pipe[0];	// This gets child's stdout.
	close(stdout_pipe[1]);
	child_stderr = stderr_pipe[0];
	close(stderr_pipe[1]);
cout << "Child_stdout is " << child_stdout << ", Child_stderr is " <<
		child_stderr << endl;
	stdout_tag = gdk_input_add(child_stdout,
			GDK_INPUT_READ, Read_from_child, this);
	stderr_tag = gdk_input_add(child_stderr,
			GDK_INPUT_READ, Read_from_child, this);
	return true;
	}

