/**
 **	Compile.cc - Run usecode compiler and show results.
 **
 **	Written: 10/08/02 - JSF
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string>
#include "studio.h"
#include "exult_constants.h"
#include "utils.h"
#include "execbox.h"

using std::cout;
using std::endl;
using std::string;

/*
 *	"Compile" button
 */
C_EXPORT void on_compile_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->compile();
	}

/*
 *	"Halt" button.
 */
C_EXPORT void on_halt_compile_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->halt_compile();
	}

/*
 *	Called when UCC is done.
 */

void Ucc_done
	(
	int exit_code,
	Exec_box *box,			// Box that called this.
	gpointer user_data		// Not used.
	)
	{
	if (exit_code == 0)		// Success?
		{
		ExultStudio::get_instance()->reload_usecode();
		box->add_message("Reloaded usecode\n");
		}
	}

/*
 *	Open the compile window.
 */

void ExultStudio::open_compile_window
	(
	)
	{
	if (!compilewin)		// First time?
		{
		compilewin = glade_xml_get_widget( app_xml, "compile_win" );
		compile_box = new Exec_box(
			GTK_TEXT(
			    glade_xml_get_widget(app_xml, "compile_msgs")),
			GTK_STATUSBAR(
			    glade_xml_get_widget(app_xml, "compile_status")),
			Ucc_done, 0);
		}
	gtk_widget_show(compilewin);
	}

/*
 *	Compile.
 */

void ExultStudio::compile
	(
	bool if_needed			// Means check timestamps.
	)
	{
					// Get source (fixed, for now).
	string source("<PATCH>/usecode.uc");
	source = get_system_path(source);
	string obj("<PATCH>/usecode");
	obj = get_system_path(obj);
	if (!U7exists(source))
		{
		if (!if_needed)
			EStudio::Alert("Source '%s' doesn't exist",
							source.c_str());
		return;			// No source.
		}
					// ++++++Check timestamps.
	open_compile_window();		// Make sure it's open.
	const char *argv[8];		// Set up args.
	argv[0] = "ucc";		// Program to run.
	argv[1] = "-o";			// Specify output.
	argv[2] = obj.c_str();
	argv[3] = source.c_str();	// What to compile.
					// ++++++Which game type (SI/BG)?
	argv[4] = 0;			// NULL.
	if (!compile_box->exec("ucc", (char **) argv))
		EStudio::Alert("Error executing usecode compiler ('ucc')");
	}

/*
 *	Halt compilation.
 */

void ExultStudio::halt_compile
	(
	)
	{
	if (compile_box)
		compile_box->kill_child();
	}
