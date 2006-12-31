/**
 **	Ucbrowse.cc - Browse usecode functions.
 **
 **	Written: Nov. 19, 2006 - JSF
 **/

/*
Copyright (C) 2001-2006 The Exult Team

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

#include "studio.h"
#include "ucbrowse.h"
#include "ucsymtbl.h"
#include "utils.h"

using std::ifstream;

/*	Columns in our table. */
enum { NAME_COL, NUM_COL, TYPE_COL, N_COLS };

/*
 *	Open browser window.
 */

const char *ExultStudio::browse_usecode
	(
	)
	{
	if (!ucbrowsewin)			// First time?
		{
		ucbrowsewin = new Usecode_browser();
		set_toggle("view_uc_functions", true);
		set_toggle("view_uc_classes", true);
		ucbrowsewin->setup_list();
		}
	ucbrowsewin->show(true);
	while (GTK_WIDGET_VISIBLE(ucbrowsewin->get_win()))	// Spin.
		gtk_main_iteration();	// (Blocks).
	const char *choice = ucbrowsewin->get_choice();
	return choice;
	}

/*
 *	Usecode_browser window's okay button.
 */
C_EXPORT void on_usecodes_ok_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
			GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->okay();
	}

/*
 *	Row was double-clicked.
 */
C_EXPORT void on_usecodes_treeview_row_activated
	(
	GtkTreeView *treeview,
        GtkTreePath *path,
        GtkTreeViewColumn *column,
        gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(treeview))));
	ucb->okay();
	}

/*
 *	Usecode_browser window's cancel button.
 */
C_EXPORT void on_usecodes_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
			GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->cancel();
	}

/*
 *	Usecode_browser window's X button.
 */
C_EXPORT gboolean on_usecodes_dialog_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) 
				gtk_object_get_user_data(GTK_OBJECT(widget));
	
	ucb->cancel();
	return TRUE;
	}

/*
 *	View classes/functions toggled.
 */
C_EXPORT void on_view_uc_classes_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->setup_list();
	}
C_EXPORT void on_view_uc_functions_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->setup_list();
	}

/*
 *	Create usecode browser window.
 */

Usecode_browser::Usecode_browser
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GladeXML *app_xml = studio->get_xml();
	win = glade_xml_get_widget(app_xml, "usecodes_dialog");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	string ucname = get_system_path("<PATCH>/usecode");
	if (!U7exists(ucname.c_str()))
		{
		ucname = get_system_path("<STATIC>/usecode");
		if (!U7exists(ucname.c_str()))
			ucname = "";
		}
	studio->set_entry("usecodes_file", ucname.c_str());
	/*
	 *	Set up table.
	 */
	model = gtk_tree_store_new (
		N_COLS,
		G_TYPE_STRING,		// Name.
		G_TYPE_STRING,		// Number.
		G_TYPE_STRING);		// Type:  function, class.
					// Create view, and set our model.
	tree = glade_xml_get_widget(app_xml, "usecodes_treeview");
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));;
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Name", renderer, "text", NAME_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes(
			"Number", renderer, "text", NUM_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes(
			"Type", renderer, "text", TYPE_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);
	gtk_widget_show(tree);
	}

/*
 *	Delete.
 */

Usecode_browser::~Usecode_browser
	(
	)
	{
	g_object_unref(model);
	gtk_widget_destroy(win);
	}

/*
 *	Show/hide.
 */

void Usecode_browser::show
	(
	bool tf
	)
	{
	if (tf)
		gtk_widget_show(win);
	else
		gtk_widget_hide(win);
	}

/*
 *	"Okay" button.
 */

void Usecode_browser::okay
	(
	)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	choice = "";
  	/* This will only work in single or browse selection mode! */
	GtkTreeSelection *selection = 
		gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
  	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    		gchar *name;
    		gtk_tree_model_get(model, &iter, NAME_COL, &name, -1);
		if (name[0])
			choice = name;
    		g_free(name);
		if (choice == "") {	// No name? Get number.
	    		gtk_tree_model_get(model, &iter, NUM_COL, &name, -1);
			choice = name;
			g_free(name);
		}
		g_print ("selected row is: %s\n", choice.c_str());
  	}
	show(FALSE);
}

/*
 *	Set up list of usecode entries.
 */

void Usecode_browser::setup_list
	(
	)
{
	ExultStudio *studio = ExultStudio::get_instance();
	const char *ucfile = studio->get_text_entry("usecodes_file");
	ifstream in;
	U7open(in, ucfile);
	Usecode_symbol_table symtbl;
	long magic = Read4(in);		// Test for symbol table.
	if (!in.good()) {
		EStudio::Alert("Error reading '%s'.", ucfile);
		return;
	}
	if (magic != UCSYMTBL_MAGIC0 || (magic = Read4(in)) 
							!= UCSYMTBL_MAGIC1)
		return;
	symtbl.read(in);
	gtk_tree_store_clear(model);
	bool show_funs = studio->get_toggle("view_uc_functions");
	bool show_classes = studio->get_toggle("view_uc_classes");
	const Usecode_symbol_table::Syms_vector& syms = symtbl.get_symbols();
	Usecode_symbol_table::Syms_vector::const_iterator siter;
	for (siter = syms.begin(); siter != syms.end(); ++siter) {
		Usecode_symbol *sym = *siter;
		Usecode_symbol::Symbol_kind kind = sym->get_kind();
		char *kindstr = 0;
		const char *nm = sym->get_name();
		if (!nm[0])
			continue;
		switch (kind) {
		case Usecode_symbol::fun_defined:
		case Usecode_symbol::shape_fun:
			if (!show_funs)
				continue;
			kindstr = "Function";
			break;
		case Usecode_symbol::class_scope:
			if (!show_classes)
				continue;
			kindstr = "Class";
			break;
		default:
			continue;
		}
		char num[20];
		sprintf(num, "%05xH", sym->get_val());
		GtkTreeIter iter;
		gtk_tree_store_append(model, &iter, NULL);
		gtk_tree_store_set(model, &iter, NAME_COL, nm,
			NUM_COL, num, TYPE_COL, kindstr, -1);
	}
}

