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


using std::vector;
using std::ios;
using std::string;
using std::cout;
using std::endl;

/*
 *	Create an empty group.
 */

Shape_group::Shape_group
	(
	char *nm,			// Name.  Copied.
	Shape_group_file *f
	) : name(nm), file(f)
	{
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
	Flex::write_header(out, "ExultStudio shape groups", cnt);
	unsigned char *table = new unsigned char[2*cnt*4];
	uint8 *tptr = table;
	int i;				// Write each group.
	for (i = 0; i < cnt; i++)
		{
		Write4(tptr, out.tellp());
		Shape_group *grp = groups[i];
		const char *nm = grp->get_name();
		int sz = grp->size();
					// Name, #entries, entries(2-bytes).
		long len = strlen(nm) + 1 + 2 + 2*sz;
		Write4(tptr, len);
		unsigned char *buf = new unsigned char[len];
		strcpy((char *) buf, nm);
		unsigned char *ptr = buf + strlen(nm) + 1;
		Write2(ptr, sz);	// # entries.
		for (vector<int>::iterator it = grp->begin();
						it != grp->end(); it++)
			Write2(ptr, *it);
		assert(ptr - buf == len);
		out.write(reinterpret_cast<char *>(buf), len);	// Write out to file.
		delete buf;
		}
	out.seekp(0x80, ios::beg);	// Write table.
	out.write(reinterpret_cast<char*>(table), 2*cnt*4);
	delete [] table;
	out.flush();
	bool result = out.good();
	if (!result)			// ++++Better error system needed??
		throw file_write_exception(patchname.c_str());
	out.close();
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
C_EXPORT gboolean
on_group_list_select_row		(GtkCList	*clist,
					 gint		row,
					 gint		column,
					 GdkEventButton	*event,
					 gpointer	 user_data)
{
	ExultStudio::get_instance()->setup_group_controls();
}

C_EXPORT gboolean
on_group_list_unselect_row		(GtkCList	*clist,
					 gint		row,
					 gint		column,
					 GdkEventButton	*event,
					 gpointer	 user_data)
{
	ExultStudio::get_instance()->setup_group_controls();
}

C_EXPORT void
on_group_list_row_move			(GtkCList	*clist,
					 gint		src_row,
					 gint		dest_row,
					 gpointer	 user_data)
{
	ExultStudio::get_instance()->move_group(src_row, dest_row);
}

C_EXPORT gboolean
on_group_list_button_press_event	(GtkCList	*clist,
					 GdkEventButton	*event,
					 gpointer	 user_data)
{
	if (event->type == GDK_2BUTTON_PRESS)
		{
		cout << "Double-clicked" << endl;
		//++++++++++++
		return true;
		}
	return false;
}

/*
 *	Initialize the list of shape groups for the file being shown in the
 *	browser.
 */

void ExultStudio::setup_groups
	(
	const char *fname		// Base filename.
	)
	{
	string name(fname);		// Start with shapes filename.
	name += ".grp";
	delete groups;			// Delete old & create new.
	groups = new Shape_group_file(name.c_str());
	GtkCList *clist = GTK_CLIST(
				glade_xml_get_widget(app_xml, "group_list"));

	gtk_clist_clear(clist);		// Clear out list.
	gtk_clist_freeze(clist);
	int cnt = groups->size();	// Add groups from file.
	for (int i = 0; i < cnt; i++)
		{
		Shape_group *grp = groups->get(i);
		char *nm0 = g_strdup(grp->get_name());
		gtk_clist_append(clist, &nm0);
		g_free(nm0);
		}
	gtk_clist_thaw(clist);
					// Enable reordering.
	gtk_clist_set_reorderable(clist, true);
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
	GtkCList *clist = GTK_CLIST(
				glade_xml_get_widget(app_xml, "group_list"));
	GList *list = clist->selection; 
	if (list)
		{
		int row = (int) list->data;
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
	if (!groups)
		return;
	GtkCList *clist = GTK_CLIST(
				glade_xml_get_widget(app_xml, "group_list"));
	char *nm = get_text_entry("groups_new_name");
	if (nm)
		{
		groups->add(new Shape_group(nm, groups));
		gtk_clist_append(clist, &nm);
		}
	set_entry("groups_new_name", "");
	}

void ExultStudio::del_group
	(
	)
	{
//	GtkCList *clist = GTK_CLIST(
//				glade_xml_get_widget(app_xml, "group_list"));
	//++++++++++
	}

/*
 *	Move a group in the list.
 */

void ExultStudio::move_group
	(
	int src_row,
	int dest_row
	)
	{
	if (!groups)
		return;
	GtkCList *clist = GTK_CLIST(
				glade_xml_get_widget(app_xml, "group_list"));
//	cout << "Row " << src_row << " moved to row " << dest_row << endl;
	Shape_group *grp = groups->get(src_row);
	groups->remove(src_row, false);	// Remove from old pos.
	groups->insert(grp, dest_row);	// Put into new spot.
	}
