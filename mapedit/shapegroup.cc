/**
 **	A group of shapes/chunks that can be used as a 'palette'.
 **
 **	Written: 1/22/02 - JSF
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

#include <cassert>
#include <fstream>
#include <gdk/gdkkeysyms.h>
#include "shapegroup.h"
#include "Flex.h"
#include "utils.h"
#include "exceptions.h"
#include "studio.h"
#include "objbrowse.h"
#include "shapefile.h"

using std::vector;
using std::ios;
using std::string;
using std::cout;
using std::endl;
using EStudio::Alert;

/*
 *	Create an empty group.
 */

Shape_group::Shape_group
	(
	const char *nm,			// Name.  Copied.
	Shape_group_file *f
	) : name(nm), file(f)
	{
	}

/*
 *	Remove i'th entry.
 */

void Shape_group::del
	(
	int i
	)
	{ 
	assert(i >= 0 && i < size());
	std::vector<int>::erase(begin() + i); 
	file->modified = true;
	}

/*
 *	Swap two entries.
 */

void Shape_group::swap
	(
	int i				// Lower one.
	)
	{
	int x0 = (*this)[i];
	(*this)[i] = (*this)[i + 1];
	(*this)[i + 1] = x0;
	file->modified = true;
	}

/*
 *	Add a new entry if not already there.
 */

void Shape_group::add
	(
	int id
	)
	{
	for (vector<int>::const_iterator it = begin(); it != end(); ++it)
		if ((*it) == id)
			return;		// Already there.
	push_back(id);
	file->modified = true;
	}

/*
 *	Init. and read in (if it exists) a groups file.
 */

Shape_group_file::Shape_group_file
	(
	const char *nm			// Basename.
	) : name(nm), modified(false)
	{
	Flex *flex = 0;
	std::string patchname = "<PATCH>/" + name;
	std::string staticname = "<STATIC>/" + name;
	if (U7exists(patchname))	// First try 'patch' directory.
		flex = new Flex(patchname);
	else if (U7exists(staticname))
		flex = new Flex(staticname);
	if (flex)			// Exists?
		{
		int cnt = flex->number_of_objects();
		for (int i = 0; i < cnt; i++)
			{		// Get each group.
			std::size_t len;
			char *buf = flex->retrieve(i, len);
			char *gname = buf;	// Starts with name.
			unsigned char *ptr = (unsigned char *)
						gname + strlen(gname) + 1;
			int sz = Read2(ptr);	// Get # entries.
			assert ((len - ((char *) ptr - buf))/2 == sz);
			Shape_group *grp = new Shape_group(gname, this);
			grp->reserve(sz);
			for (int j = 0; j < sz; j++)
				grp->push_back(Read2(ptr));
			groups.push_back(grp);
			delete buf;
			}
		delete flex;
		}
	}

/*
 *	Search for a group with a given name.
 *
 *	Output:	Index if found, else -1.
 */

int Shape_group_file::find
	(
	const char *nm
	)
	{
	for (vector<Shape_group *>::const_iterator it = groups.begin();
						it != groups.end(); ++it)
		if ((*it)->name == nm)
			return (it - groups.begin());
	return -1;
	}

/*
 *	Clean up.
 */

Shape_group_file::~Shape_group_file
	(
	)
	{
	for (vector<Shape_group *>::iterator it = groups.begin();
						it != groups.end(); ++it)
		delete (*it);		// Delete each group.
	}

/*
 *	Remove and delete a group.
 */

void Shape_group_file::remove
	(
	int index,
	bool del			// True to delete the group.
	)
	{
	modified = true;
	assert(index >= 0 && index < groups.size());
	Shape_group *grp = groups[index];
	groups.erase(groups.begin() + index);
	if (del)
		delete grp;
	}

/*
 *	Write out to the 'patch' directory.
 */

void Shape_group_file::write
	(
	)
	{
	std::ofstream out;
	std::string patchname = "<PATCH>/" + name;
	U7open(out, patchname.c_str());
	int cnt = groups.size();	// # groups.
	Flex_writer gfile(out, "ExultStudio shape groups", cnt);
	int i;				// Write each group.
	for (i = 0; i < cnt; i++)
		{
		Shape_group *grp = groups[i];
		const char *nm = grp->get_name();
		int sz = grp->size();
					// Name, #entries, entries(2-bytes).
		long len = strlen(nm) + 1 + 2 + 2*sz;
		unsigned char *buf = new unsigned char[len];
		strcpy((char *) buf, nm);
		unsigned char *ptr = buf + strlen(nm) + 1;
		Write2(ptr, sz);	// # entries.
		for (vector<int>::iterator it = grp->begin();
						it != grp->end(); it++)
			Write2(ptr, *it);
		assert(ptr - buf == len);
					// Write out to file.
		out.write(reinterpret_cast<char *>(buf), len);
		delete buf;
		gfile.mark_section_done();
		}
	if (!gfile.close())
		Alert("Error writing '%s'", patchname.c_str());
	modified = false;
	}

/*
 *	Get the tree row # (assuming just a simple list) for a given tree-
 *	model position.
 */

static int Get_tree_row
	(
	GtkTreePath *path
	)
	{
	gchar *str = gtk_tree_path_to_string(path);
	int row = atoi(str);
	g_free(str);
	return row;
	}
static int Get_tree_row
	(
	GtkTreeModel *model,
	GtkTreeIter *iter		// Position we want.
	)
	{
	GtkTreePath *path = gtk_tree_model_get_path(model, iter);
	int row = Get_tree_row(path);
	gtk_tree_path_free(path);
	return row;
	}

/*
 *	Group buttons:
 */
C_EXPORT void
on_groups_add_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->add_group();
}

C_EXPORT void
on_groups_del_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->del_group();
}

C_EXPORT gboolean
on_groups_new_name_key_press		(GtkEntry	*entry,
					 GdkEventKey	*event,
					 gpointer	 user_data)
{
	if (event->keyval == GDK_Return)
		{
		ExultStudio::get_instance()->add_group();
		return TRUE;
		}
	return FALSE;			// Let parent handle it.
}

/*
 *	Groups list signals:
 */
C_EXPORT void
on_group_list_cursor_changed		(GtkTreeView	*tview)
{
	ExultStudio::get_instance()->setup_group_controls();
}

void
on_group_list_row_inserted		(GtkTreeModel *model,
					 GtkTreePath *path,
					 GtkTreeIter *iter,
					 gpointer user_data)
{
	ExultStudio::get_instance()->groups_changed(model, path, iter);
}

void
on_group_list_row_changed		(GtkTreeModel *model,
					 GtkTreePath *path,
					 GtkTreeIter *iter,
					 gpointer user_data)
{
	ExultStudio::get_instance()->groups_changed(model, path, iter, true);
}

void
on_group_list_row_deleted		(GtkTreeModel *model,
					 GtkTreePath *path,
					 gpointer user_data)
{
	ExultStudio::get_instance()->groups_changed(model, path, 0);
}

C_EXPORT void
on_group_list_row_activated		(GtkTreeView	*treeview,
					 GtkTreePath	*path,
					 GtkTreeViewColumn *column,
					 gpointer	 user_data)
{
	ExultStudio::get_instance()->open_group_window();
}

/* columns */
enum
{
  GRP_FILE_COLUMN = 0,
  GRP_GROUP_COLUMN,
  GRP_NUM_COLUMNS
};

/*
 *	Initialize the list of shape groups for the file being shown in the
 *	browser.
 */

void ExultStudio::setup_groups
	(
	)
	{
	Shape_group_file *groups = curfile->get_groups();
	if (!groups)			// No groups?
		{
		set_visible("groups_frame", FALSE);
		return;
		}
	GtkTreeView *tview = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "group_list"));
	GtkTreeModel *oldmod = gtk_tree_view_get_model(tview);
	GtkTreeStore *model;
	gulong addsig = 0, delsig = 0, chgsig = 0;
	if (!oldmod)			// Create model first time.
		{
  		model = gtk_tree_store_new(GRP_NUM_COLUMNS, 
						G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(tview, GTK_TREE_MODEL(model));
		g_object_unref(model);
					// Create column.
	  	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		g_object_set (renderer, "xalign", 0.0, NULL);
		gint col_offset = gtk_tree_view_insert_column_with_attributes(
				tview,
				-1, "Names",
				renderer, "text",
				GRP_FILE_COLUMN, NULL);
		GtkTreeViewColumn *column = gtk_tree_view_get_column(tview, 
							col_offset - 1);
		gtk_tree_view_column_set_clickable(column, TRUE);
		addsig = g_signal_connect(G_OBJECT(model), "row-inserted",
			GTK_SIGNAL_FUNC(on_group_list_row_inserted), this);
					// Store signal id with model.
		g_object_set_data(G_OBJECT(model), "row-inserted",
							(gpointer) addsig);
		delsig = g_signal_connect(G_OBJECT(model), "row-deleted",
			GTK_SIGNAL_FUNC(on_group_list_row_deleted), this);
		g_object_set_data(G_OBJECT(model), "row-deleted",
							(gpointer) delsig);
		chgsig = g_signal_connect(G_OBJECT(model), "row-changed",
			GTK_SIGNAL_FUNC(on_group_list_row_changed), this);
		g_object_set_data(G_OBJECT(model), "row-changed",
							(gpointer) delsig);
		}
	else
		{
		model = GTK_TREE_STORE(oldmod);
		addsig = (gulong) g_object_get_data(G_OBJECT(model), 
							"row-inserted");
		delsig = (gulong) g_object_get_data(G_OBJECT(model), 
							"row-deleted");
		chgsig = (gulong) g_object_get_data(G_OBJECT(model), 
							"row-changed");
		}
					// Block this signal during creation.
	g_signal_handler_block(model, addsig);
	g_signal_handler_block(model, delsig);
	g_signal_handler_block(model, chgsig);
	gtk_tree_store_clear(model);
	set_visible("groups_frame", TRUE);
	gtk_tree_view_set_reorderable(tview, TRUE);
	int cnt = groups->size();	// Add groups from file.
	GtkTreeIter iter;
	for (int i = 0; i < cnt; i++)
		{
		Shape_group *grp = groups->get(i);
		gtk_tree_store_append(model, &iter, NULL);
		gtk_tree_store_set(model, &iter,
			GRP_FILE_COLUMN, grp->get_name(),
			GRP_GROUP_COLUMN, grp,
			-1);
		}
	gtk_tree_view_set_rules_hint(tview, TRUE);
	g_signal_handler_unblock(model, addsig);
	g_signal_handler_unblock(model, delsig);
	g_signal_handler_unblock(model, chgsig);
	setup_group_controls();		// Enable/disable the controls.
	}

/*
 *	Enable/disable the controls based on whether there's a selection.
 */

void ExultStudio::setup_group_controls
	(
	)
	{
	set_visible("groups_frame", true);
	GtkTreeView *tview = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "group_list"));
	GtkTreeSelection *list = gtk_tree_view_get_selection(tview);
	if (list)
		{
//		int row = (int) list->data;
//		set_sensitive("groups_open", true);
		set_sensitive("groups_del", true);
//		set_sensitive("groups_up_arrow", row > 0);
//		set_sensitive("groups_down_arrow", row < clist->rows);
		}
	else
		{
//		set_sensitive("groups_open", false);
		set_sensitive("groups_del", false);
//		set_sensitive("groups_up_arrow", false);
//		set_sensitive("groups_down_arrow", false);
		}
	}

/*
 *	Add/delete a new group.
 */

void ExultStudio::add_group
	(
	)
	{
	if (!curfile)
		return;
	GtkTreeView *tview = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "group_list"));
  	GtkTreeStore *model = GTK_TREE_STORE(gtk_tree_view_get_model(tview));
	const char *nm = get_text_entry("groups_new_name");
	Shape_group_file *groups = curfile->get_groups();
					// Make sure name isn't already there.
	if (nm && *nm && groups->find(nm) < 0)
		{
		Shape_group *grp = new Shape_group(nm, groups);
		GtkTreeIter iter;
		gtk_tree_store_append(model, &iter, NULL);
		gtk_tree_store_set(model, &iter,
			GRP_FILE_COLUMN, nm,
			GRP_GROUP_COLUMN, grp,
			-1);
		}
	set_entry("groups_new_name", "");
	}

void ExultStudio::del_group
	(
	)
	{
	if (!curfile)
		return;
	GtkTreeView *tview = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "group_list"));
	GtkTreeSelection *list = gtk_tree_view_get_selection(tview);
	if (!list)
		return;
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(list, &model, &iter))
		return;
	int row = Get_tree_row(model, &iter);
	Shape_group_file *groups = curfile->get_groups();
	Shape_group *grp = groups->get(row);
	string msg("Delete group '");
	msg += grp->get_name();
	msg += "'?";
	int choice = prompt(msg.c_str(), "Yes", "No");
	if (choice != 0)		// Yes?
		return;
	delete groups->get(row);	// Delete the group.
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
					// Close the group's windows.
	vector<GtkWindow*> toclose;
	vector<GtkWindow*>::const_iterator it;
	for (it = group_windows.begin(); it != group_windows.end(); ++it)
		{
		Object_browser *chooser = (Object_browser *) 
			gtk_object_get_data(GTK_OBJECT(*it), "browser");
		if (chooser->get_group() == grp)
					// A match?
			toclose.push_back(*it);
		}
	for (it = toclose.begin(); it != toclose.end(); ++it)
		close_group_window(GTK_WIDGET(*it));
	}

/*
 *	A group has been inserted or deleted from the list.
 */

void ExultStudio::groups_changed
	(
	GtkTreeModel *model,
	GtkTreePath *path,		// Position.
	GtkTreeIter *loc,		// Insert/change location, or 0.
	bool value			// True if value changed at loc.
	)
	{
	if (!curfile)
		return;
	Shape_group_file *groups = curfile->get_groups();
	int row = Get_tree_row(path);
	if (!loc)
		groups->remove(row, false);
	else
		{
		void *grpaddr = 0;
		gtk_tree_model_get(model, loc, GRP_GROUP_COLUMN, &grpaddr,
								-1);
		Shape_group *grp = (Shape_group *) grpaddr;
		if (value)		// Changed?
			groups->set(grp, row);
		else
			groups->insert(grp, row);
		}
	}

/*
 *	Events on 'group' window:
 */
/*
 *	Npc window's close button.
 */
C_EXPORT gboolean on_group_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_group_window(widget);
	return TRUE;
	}
C_EXPORT void
on_group_up_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	Object_browser *chooser = (Object_browser *)gtk_object_get_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))), 
							"browser");
	Shape_group *grp = chooser->get_group();
	int i = chooser->get_selected();
	if (grp && i > 0)		// Moving item up.
		{
		grp->swap(i - 1);
		ExultStudio::get_instance()->update_group_windows(grp);
		}
}
C_EXPORT void
on_group_down_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	Object_browser *chooser = (Object_browser *)gtk_object_get_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))), 
							"browser");
	Shape_group *grp = chooser->get_group();
	int i = chooser->get_selected();
	if (grp && i < grp->size() - 1)	// Moving down.
		{
		grp->swap(i);
		ExultStudio::get_instance()->update_group_windows(grp);
		}
}
C_EXPORT void
on_group_shape_remove_clicked		(GtkToggleButton *button,
					 gpointer	  user_data)
{
	Object_browser *chooser = (Object_browser *)gtk_object_get_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))), 
							"browser");
	Shape_group *grp = chooser->get_group();
	int i = chooser->get_selected();
	if (grp && i >= 0)
		{
		grp->del(i);
		ExultStudio::get_instance()->update_group_windows(grp);
		}
}

/*
 *	Open a 'group' window for the currently selected group.
 */

void ExultStudio::open_group_window
	(
	)
	{
	GtkTreeView *tview = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "group_list"));
	GtkTreeSelection *list = gtk_tree_view_get_selection(tview);
	if (!list || !curfile || !vgafile || !palbuf)
		return;
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(list, &model, &iter))
		return;
	int row = Get_tree_row(model, &iter);
	Shape_group_file *groups = curfile->get_groups();
	Shape_group *grp = groups->get(row);
	GladeXML *xml = glade_xml_new(glade_path, "group_window", NULL);
	glade_xml_signal_autoconnect(xml);
	GtkWidget *grpwin = glade_xml_get_widget(xml, "group_window");
	Object_browser *chooser = curfile->create_browser(vgafile,palbuf, grp);
					// Set xml as data on window.
	gtk_object_set_data(GTK_OBJECT(grpwin), "xml", xml);
	gtk_object_set_data(GTK_OBJECT(grpwin), "browser", chooser);
					// Set window title, name field.
	string title("Exult Group:  ");
	title += grp->get_name();
	gtk_window_set_title(GTK_WINDOW(grpwin), title.c_str());
	GtkWidget *field = glade_xml_get_widget(xml, "group_name");
	if (field)
		gtk_entry_set_text(GTK_ENTRY(field), grp->get_name());
					// Attach browser.
	GtkWidget *browser_box = glade_xml_get_widget(xml, "group_shapes" );
	gtk_widget_show(browser_box);
	gtk_box_pack_start(GTK_BOX(browser_box), chooser->get_widget(), 
								TRUE, TRUE, 0);
					// Auto-connect doesn't seem to work.
	gtk_signal_connect (GTK_OBJECT(grpwin), "delete_event",
                      GTK_SIGNAL_FUNC (on_group_window_delete_event),
                      this);
	group_windows.push_back(GTK_WINDOW(grpwin));
	gtk_widget_show(grpwin);
	}

/*
 *	Close a 'group' window.
 */

void ExultStudio::close_group_window
	(
	GtkWidget *grpwin
	)
	{
					// Remove from list.
	for (vector<GtkWindow*>::iterator it = group_windows.begin();
					it != group_windows.end(); ++it)
		{
		if (*it == GTK_WINDOW(grpwin))
			{
			group_windows.erase(it);
			break;
			}
		}
	GladeXML *xml = (GladeXML *) gtk_object_get_data(GTK_OBJECT(grpwin), 
							"xml");
	Object_browser *chooser = (Object_browser *) gtk_object_get_data(
			GTK_OBJECT(grpwin), "browser");
	delete chooser;
	gtk_widget_destroy(grpwin);
	g_object_unref(G_OBJECT(xml));
	}

/*
 *	Save all modified group files.
 */

void ExultStudio::save_groups
	(
	)
	{
	if (!files)
		return;
	int cnt = files->size();
	for (int i = 0; i < cnt; i++)	// Check each file.
		{
		Shape_file_info *info = (*files)[i];
		Shape_group_file *gfile = info->get_groups();
		if (gfile && gfile->is_modified())
			gfile->write();
		}
	}

/*
 *	Return TRUE if any groups have been modified.
 */

bool ExultStudio::groups_modified
	(
	)
	{
	if (!files)
		return false;
	int cnt = files->size();
	for (int i = 0; i < cnt; i++)	// Check each file.
		{
		Shape_file_info *info = (*files)[i];
		Shape_group_file *gfile = info->get_groups();
		if (gfile && gfile->is_modified())
			return true;
		}
	return false;
	}

/*
 *	Update all windows showing a given group.
 */

void ExultStudio::update_group_windows
	(
	Shape_group *grp		// Group, or 0 for all.
	)
	{
	for (vector<GtkWindow*>::const_iterator it = group_windows.begin();
					it != group_windows.end(); ++it)
		{
		Object_browser *chooser = (Object_browser *) 
			gtk_object_get_data(GTK_OBJECT(*it), "browser");
		if (!grp || chooser->get_group() == grp)
			{		// A match?
			chooser->render();
			chooser->show();
			}
		}
	}

