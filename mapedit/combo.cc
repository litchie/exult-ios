/**
 **	Combo.cc - A combination of multiple objects.
 **
 **	Written: 4/26/02 - JSF
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

#include <gdk/gdkkeysyms.h>
#include "studio.h"
#include "combo.h"
#include "exult_constants.h"
#include "shapevga.h"
#include "shapefile.h"
#include "ibuf8.h"
#include "objserial.h"
#include "shapegroup.h"
#include "Flex.h"
#include "u7drag.h"

const int border = 2;			// Border at bottom, sides of each
					//   combo in browser.

/*
 *	Open combo window (if not already open).
 */

C_EXPORT void on_new_combo1_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_combo_window();
	}
void ExultStudio::open_combo_window
	(
	)
	{
	if (combowin && combowin->is_visible())
		return;			// Already open.
	if (!vgafile)
		{
		EStudio::Alert("'shapes.vga' file isn't present");
		return;
		}
	Shapes_vga_file *svga = (Shapes_vga_file *) vgafile->get_ifile();
	delete combowin;		// Delete old (svga may have changed).
	combowin = new Combo_editor(svga, palbuf);
	combowin->show(true);
					// Set edit-mode to pick.
	GtkWidget *mitem = glade_xml_get_widget(app_xml, "pick_for_combo1");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem), TRUE);
	}

/*
 *	Callbacks for "Combo" editor window.
 */
C_EXPORT gint on_combo_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(widget))));
	combo->render(&event->area);
	return TRUE;
	}

C_EXPORT void
on_combo_remove_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->remove();
}

C_EXPORT void
on_combo_apply_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->save();
}

C_EXPORT void
on_combo_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
							GTK_OBJECT(win));
	combo->save();
	gtk_widget_hide(win);
}

C_EXPORT void
on_combo_locx_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->set_position();
}

C_EXPORT void
on_combo_locy_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->set_position();
}

C_EXPORT void
on_combo_locz_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->set_position();
}

C_EXPORT void
on_combo_order_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(button))));
	combo->set_order();
}

/*
 *	Mouse events in draw area.
 */
C_EXPORT gint on_combo_draw_button_press_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Combo_chooser.
	)
	{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(widget))));
	return combo->mouse_press(event);
	}

/*
 *	Create empty combo.
 */

Combo::Combo
	(
	Shapes_vga_file *svga
	) : shapes_file(svga),
	    starttx(c_num_tiles), startty(c_num_tiles), xtiles(0), ytiles(0), 
	    ztiles(0), hot_index(-1)
	{
					// Read info. the first time.
	shapes_file->read_info(false, true);//+++++BG?
	}

/*
 *	Copy another.
 */

Combo::Combo
	(
	const Combo& c2
	) : shapes_file(c2.shapes_file), starttx(c2.starttx),
	    startty(c2.startty), xtiles(c2.xtiles), ytiles(c2.ytiles),
	    ztiles(c2.ztiles), hot_index(c2.hot_index), name(c2.name)
	{
	for (vector<Combo_member *>::const_iterator it = c2.members.begin();
					it != c2.members.end(); ++it)
		{
		Combo_member *m = *it;
		add(m->tx, m->ty, m->tz, m->shapenum, m->framenum);
		}
	}

/*
 *	Clean up.
 */

Combo::~Combo
	(
	)
	{
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		delete *it;
	}

/*
 *	Add a new object.
 */

void Combo::add
	(
	int tx, int ty, int tz,		// Location rel. to top-left.
	int shnum, int frnum		// Shape.
	)
	{
					// Look for identical shape, pos.
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		{
		Combo_member *m = *it;
		if (tx == m->tx && ty == m->ty && tz == m->tz &&
		    shnum == m->shapenum && frnum == m->framenum)
			return;		// Don't add same one twice.
		}
#if 0	/* This doesn't work right. */
	if (members.size() != 0)	// Not the first?
		{			// Push within current range.
		if (tx < starttx - 16)
			tx = starttx - 16;
		else if (tx > starttx + 16)
			tx = starttx + 16;
		if (ty < startty - 16)
			ty = startty - 16;
		else if (ty > startty + 16)
			ty = startty + 16;
		}
#endif
	Combo_member *memb = new Combo_member(tx, ty, tz, shnum, frnum);
	members.push_back(memb);
					// Figure visible top-left tile.
	Shape_info& info = shapes_file->get_info(shnum);
	int xtiles = info.get_3d_xtiles(frnum),
	    ytiles = info.get_3d_ytiles(frnum),
	    ztiles = info.get_3d_height();
	int vtx = tx - xtiles - (tz + ztiles + 1)/2, 
	    vty = ty - ytiles - (tz + ztiles + 1)/2;
	if (vtx < starttx)		// Adjust our starting point.
		starttx = vtx;
	if (vty < startty)
		startty = vty;
	if (hot_index == -1)		// First one?
		hot_index = 0;
	}

/*
 *	Remove i'th object.
 */

void Combo::remove
	(
	int i
	)
	{
	if (i < 0 || i >= members.size())
		return;
					// Get and remove i'th entry.
	vector<Combo_member *>::iterator it = members.begin() + i;
	Combo_member *m = *it;
	members.erase(it);
	delete m;
	// +++++++Mayby re-adjust top tx, ty???
	}

/*
 *	Paint in a drawing area.
 */

void Combo::draw
	(
	Shape_draw *draw,
	int selected,			// Index of 'selected' item, or -1.
	int xoff, int yoff		// Offset within draw.
	)
	{
	int selx = -1000, sely = -1000;
	bool selfound = false;
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		{
		Combo_member *m = *it;
					// Figure pixels up-left for lift.
		int lft = m->tz*(c_tilesize/2);
					// Figure relative tile.
		int mtx = m->tx - starttx,
		    mty = m->ty - startty;
					// Hot spot:
		int x = mtx*c_tilesize - lft,
		    y = mty*c_tilesize - lft;
		Shape_frame *shape = shapes_file->get_shape(m->shapenum,
								m->framenum);
		if (!shape)
			continue;
					// But draw_shape uses top-left.
		x -= shape->get_xleft();
		y -= shape->get_yabove();
		x += xoff; y += yoff;	// Add offset within area.
		draw->draw_shape(shape, x, y);
		if (it - members.begin() == selected)
			{
			selx = x;	// Save coords for selected.
			sely = y;
			selfound = true;
			}
		}
	if (selfound)			// Now put border around selected.
		{
		Combo_member *m = members[selected];
					// FOR NOW, use color #1 ++++++++
		draw->draw_shape_outline(m->shapenum, m->framenum,
						selx, sely, 1);
		}
	}

/*
 *	Find last member in list that contains a mouse point.
 *
 *	Output:	Index of member found, or -1 if none.
 */

int Combo::find
	(
	int mx, int my			// Mouse position in draw area.
	)
	{
	int cnt = members.size();
	for (int i = cnt - 1; i >= 0; i--)
		{
		Combo_member *m = members[i];
					// Figure pixels up-left for lift.
		int lft = m->tz*(c_tilesize/2);
					// Figure relative tile.
		int mtx = m->tx - starttx,
		    mty = m->ty - startty;
		int x = mtx*c_tilesize - lft,
		    y = mty*c_tilesize - lft;
		Shape_frame *frame = shapes_file->get_shape(
						m->shapenum, m->framenum);
		if (frame && frame->has_point(mx - x, my - y))
			return i;
		}
	return -1;
	}

/*
 *	Write out.
 *
 *	Output:	Allocated buffer containing result.
 */

unsigned char *Combo::write
	(
	int& datalen			// Actual length of data in buf. is
					//   returned here.
	)
	{
	int namelen = name.length();	// Name length.
					// Room for our data + members.
	unsigned char *buf = new unsigned char[namelen + 1 + 
						7*4 + members.size()*(5*4)];
	unsigned char *ptr = buf;
	Serial_out out(ptr);
	out << name;
	out << hot_index << starttx << startty << xtiles << ytiles << ztiles;
	out << (short) members.size();	// # members to follow.
	for (std::vector<Combo_member *>::const_iterator it = members.begin();
					it != members.end(); ++it)
		{
		Combo_member *m = *it;
		out << m->tx << m->ty << m->tz << m->shapenum <<
						m->framenum;
		}
	datalen = ptr - buf;		// Return actual length.
	return buf;
	}

/*
 *	Read in.
 *
 *	Output:	->past actual data read.
 */

unsigned char *Combo::read
	(
	unsigned char *buf,
	int bufsize
	)
	{
	unsigned char *ptr = buf;
	Serial_in in(ptr);
	in << name;
	in << hot_index << starttx << startty << xtiles << ytiles << ztiles;
	short cnt;
	in << cnt;			// # members to follow.
	for (int i = 0; i < cnt; i++)
		{
		short tx, ty, tz, shapenum, framenum;
		in << tx << ty << tz << shapenum << framenum;
		add(tx, ty, tz, shapenum, framenum);
		}
	return ptr;
	}

/*
 *	Set to edit an existing combo.
 */

void Combo_editor::set_combo
	(
	Combo *newcombo,		// We'll own this.
	int findex			// File index.
	)
	{
	delete combo;
	combo = newcombo;
	file_index = findex;
	selected = -1;
	ExultStudio::get_instance()->set_entry(
			"combo_name", combo->name.c_str(), true);
	set_controls();			// No selection now.
	render();
	}

/*
 *	Create combo editor.
 */

Combo_editor::Combo_editor
	(
	Shapes_vga_file *svga,		// File containing shapes.
	unsigned char *palbuf		// Palette for drawing shapes.
	) : Shape_draw(svga, palbuf, glade_xml_get_widget(
		ExultStudio::get_instance()->get_xml(), "combo_draw")),
	    selected(-1), setting_controls(false), file_index(-1)
	{
	static bool first = true;
	combo = new Combo(svga);
	GladeXML *app_xml = ExultStudio::get_instance()->get_xml();
	win = glade_xml_get_widget(app_xml, "combo_win");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	if (first)			// Indicate the events we want.
		{
		gtk_widget_set_events(draw, GDK_EXPOSURE_MASK | 
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		    GDK_BUTTON1_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
		first = false;
		}
#if 0
	combo->add(5, 5, 0, 162, 0);	// Testing.
	combo->add(7, 7, 0, 161, 0);
#endif
	set_controls();
	}

/*
 *	Clean up.
 */

Combo_editor::~Combo_editor
	(
	)
	{
	delete combo;
	}

/*
 *	Show/hide.
 */

void Combo_editor::show
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
 *	Display.
 */

void Combo_editor::render
	(
	GdkRectangle *area		// 0 for whole draw area.
	)
	{
	Shape_draw::configure();	// Setup the first time.
					// Get dims.
	int draww = draw->allocation.width,
	    drawh = draw->allocation.height;
	GdkRectangle all;
	if (!area)
		{
		all.x = all.y = 0;
		all.width = draww;
		all.height = drawh;
		area = &all;
		}
	gdk_gc_set_clip_rectangle(drawgc, area);
	iwin->fill8(255);		// Fill with background color.
	combo->draw(this, selected);	// Draw shapes.
	Shape_draw::show(area->x, area->y, area->width, area->height);
	}

/*
 *	Set controls according to what's selected.
 */

void Combo_editor::set_controls
	(
	)
	{
	setting_controls = true;	// Avoid updating.
	ExultStudio *studio = ExultStudio::get_instance();
	Combo_member *m = combo->get(selected);
	if (!m)				// None selected?
		{
		studio->set_spin("combo_locx", 0, false);
		studio->set_spin("combo_locy", 0, false);
		studio->set_spin("combo_locz", 0, false);
		studio->set_spin("combo_order", 0, false);
		studio->set_sensitive("combo_remove", false);
		}
	else
		{
		int draww = draw->allocation.width,
		    drawh = draw->allocation.height;
		studio->set_sensitive("combo_locx", true);
		studio->set_spin("combo_locx", m->tx - combo->starttx,
							0, draww/c_tilesize);
		studio->set_sensitive("combo_locy", true);
		studio->set_spin("combo_locy", m->ty - combo->startty,
							0, drawh/c_tilesize);
		studio->set_sensitive("combo_locz", true);
		studio->set_spin("combo_locz", m->tz, 0, 15);
		studio->set_sensitive("combo_order", true);
		studio->set_spin("combo_order", selected, 0,
						combo->members.size() - 1);
		studio->set_sensitive("combo_remove", true);
		}
	setting_controls = false;
	}

/*
 *	Handle a mouse-press event.
 */

gint Combo_editor::mouse_press
	(
	GdkEventButton *event
	)
	{
	if (event->button != 1)
		return FALSE;		// Handling left-click.
					// Get mouse position, draw dims.
	int mx = (int) event->x, my = (int) event->y;
	selected = combo->find(mx, my);	// Find it (or -1 if not found).
	set_controls();
	render();
	return TRUE;
	}

/*
 *	Move the selected item within the order.
 */

void Combo_editor::set_order
	(
	)
	{
	if (setting_controls || selected < 0)
		return;
	ExultStudio *studio = ExultStudio::get_instance();
	int newi = studio->get_spin("combo_order");
	if (selected == newi)
		return;			// Already correct.
	int dir = newi > selected ? 1 : -1;
	while (newi != selected)
		{
		Combo_member *tmp = combo->members[selected + dir];
		combo->members[selected + dir] = combo->members[selected];
		combo->members[selected] = tmp;
		selected += dir;
		}
	render();
	}

/*
 *	Move the selected item to the desired position in the spin buttons.
 */

void Combo_editor::set_position
	(
	)
	{
	Combo_member *m = combo->get(selected);
	if (!m || setting_controls)
		return;
	ExultStudio *studio = ExultStudio::get_instance();
	m->tx = combo->starttx + studio->get_spin("combo_locx");
	m->ty = combo->startty + studio->get_spin("combo_locy");
	m->tz = studio->get_spin("combo_locz");
	render();
	}

/*
 *	Add an object/shape picked from Exult.
 */

void Combo_editor::add
	(
	unsigned char *data,		// Serialized object.
	int datalen
	)
	{
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, quality;
	std::string name;
	if (!Object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		quality, name))
		{
		cout << "Error decoding object" << endl;
		return;
		}
	combo->add(tx, ty, tz, shape, frame);
	render();
	}

/*
 *	Remove selected.
 */

void Combo_editor::remove
	(
	)
	{
	if (selected >= 0)
		{
		combo->remove(selected);
		selected = -1;
		set_controls();
		render();
		}
	}

/*
 *	Save combo.
 */

void Combo_editor::save
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
					// Get name from field.
	combo->name = studio->get_text_entry("combo_name");
	Flex_file_info *flex_info = dynamic_cast<Flex_file_info *>
				(studio->get_files()->create("combos.flx"));
	if (!flex_info)
		{
		EStudio::Alert("Can't open 'combos.flx'");
		return;
		}
	flex_info->set_modified();
	int len;			// Serialize.
	unsigned char *newbuf = combo->write(len);
					// Update or append file data.
	flex_info->set(file_index == -1 ? flex_info->size() : file_index, 
					(char *) newbuf, len);
	Combo_chooser *browser = dynamic_cast<Combo_chooser *>(
						studio->get_browser());
	if (browser)			// Browser open?
		file_index = browser->add(new Combo(*combo), file_index);
	}

/*
 *	Blit onto screen.
 */

void Combo_chooser::show
	(
	int x, int y, int w, int h	// Area to blit.
	)
	{
	Shape_draw::show(draw->window, x, y, w, h);
	if (selected >= 0)		// Show selected.
		{
		Rectangle b = info[selected].box;
					// Draw yellow box.
		gdk_draw_rectangle(draw->window, drawgc, FALSE, 
							b.x, b.y, b.w, b.h);
		}
	}

/*
 *	Select an entry.  This should be called after rendering
 *	the combo.
 */

void Combo_chooser::select
	(
	int new_sel
	)
	{
	if (new_sel < 0 || new_sel >= info_cnt)
		return;			// Bad value.
	selected = new_sel;
	enable_controls();
	int num = info[selected].num;
	Combo *combo = combos[num];
					// Remove prev. selection msg.
//	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Combo %d", num);
	if (combo && !combo->name.empty())
		{
		int len = strlen(buf);
		g_snprintf(buf + len, sizeof(buf) - len, 
				":  '%s'", combo->name.c_str());
		}
	gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
	}

/*
 *	Unselect.
 */

void Combo_chooser::unselect
	(
	bool need_render			// 1 to render and show.
	)
	{
	if (selected >= 0)
		{
		selected = -1;
		gtk_drag_source_unset(draw);
		if (need_render)
			{
			render();
			show();
			}
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	enable_controls();		// Enable/disable controls.
	char buf[150];			// Show new selection.
	if (info_cnt > 0)
		{
//		gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
		g_snprintf(buf, sizeof(buf), "Combos %d to %d",
			info[0].num, info[info_cnt - 1].num);
		gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
		}
	else
		{
//		gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
		gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel,
							"No combos");
		}
	}

/*
 *	Render as many combos as fit in the combo chooser window.
 */

void Combo_chooser::render
	(
	)
	{
					// Look for selected frame.
	int selcombo = -1, new_selected = -1;
	if (selected >= 0)		// Save selection info.
		selcombo = info[selected].num;
					// Remove "selected" message.
	//gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	delete [] info;			// Delete old info. list.
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Provide more than enough room.
	info = new Combo_info[256];
	info_cnt = 0;			// Count them.
					// Clear window first.
	iwin->fill8(255);		// Background color.
	int index = index0;
					// We'll always show 128x128.
	const int combow = 128, comboh = 128;
	int total_cnt = combos.size();
	int y = border;
					// Show bottom if at least 1/2 vis.
	while (index < total_cnt && y + comboh/2 + border <= winh)
		{
		int x = border;
		int cliph = y + comboh <= winh ? comboh : (winh - y);
		while (index < total_cnt && x + combow + border <= winw)
			{
			iwin->set_clip(x, y, combow, cliph);
			int combonum = group ? (*group)[index] : index;
			combos[combonum]->draw(this, -1, x, y);
			iwin->clear_clip();
					// Store info. about where drawn.
			info[info_cnt].set(combonum, x, y, combow, comboh);
			if (combonum == selcombo)
						// Found the selected combo.
				new_selected = info_cnt;
			info_cnt++;
			index++;		// Next combo.
			x += combow + border;
			}
		y += comboh + border;
		}
	if (new_selected == -1)
		unselect(false);
	else
		select(new_selected);
	}

/*
 *	Scroll to a new combo.
 */

void Combo_chooser::scroll
	(
	int newindex			// Abs. index of leftmost to show.
	)
	{
	int total = combos.size();
	if (index0 < newindex)	// Going forwards?
		index0 = newindex < total ? newindex : total;
	else if (index0 > newindex)	// Backwards?
		index0 = newindex >= 0 ? newindex : 0;
	render();
	show();
	}

/*
 *	Scroll up/down by one row.
 */

void Combo_chooser::scroll
	(
	bool upwards
	)
	{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(combo_scroll));
	float delta = adj->step_increment;
	if (upwards)
		delta = -delta;
	adj->value += delta;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	scroll((gint) adj->value);
	}

/*
 *	Someone wants the dragged combo.
 */

void Combo_chooser::drag_data_get
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	GtkSelectionData *seldata,	// Fill this in.
	guint info,
	guint time,
	gpointer data			// ->Shape_chooser.
	)
	{
	cout << "In DRAG_DATA_GET" << endl;
	Combo_chooser *chooser = (Combo_chooser *) data;
	if (chooser->selected < 0 || info != U7_TARGET_COMBOID)
		return;			// Not sure about this.
					// Get combo #.
	int num = chooser->info[chooser->selected].num;
	Combo *combo = chooser->combos[num];
					// Get enough memory.
	int cnt = combo->members.size();
	guchar *buf = new unsigned char[4 + cnt*5*4];
	guchar *ptr = buf;
	U7_combo_data *ents = new U7_combo_data[cnt];
					// Get 'hot-spot' member.
	Combo_member *hot = combo->members[combo->hot_index];
	for (int i = 0; i < cnt; i++)
		{
		Combo_member *m = combo->members[i];
		ents[i].tx = m->tx - hot->tx;
		ents[i].ty = m->ty - hot->ty;
		ents[i].tz = m->tz - hot->tz;
		ents[i].shape = m->shapenum;
		ents[i].frame = m->framenum;
		}
	int len = Store_u7_comboid(buf, cnt, ents);
#ifdef WIN32
	windragdata *wdata = (windragdata *)seldata;
	wdata->data = buf;
	wdata->id = info;
#else
					// Make us owner of xdndselection.
	gtk_selection_owner_set(widget, gdk_atom_intern("XdndSelection", 0),
								time);
					// Set data.
	gtk_selection_data_set(seldata,
			gdk_atom_intern(U7_TARGET_COMBOID_NAME, 0),
                                				8, buf, len);
#endif
	delete buf;
	delete [] ents;
	}

/*
 *	Another app. has claimed the selection.
 */

gint Combo_chooser::selection_clear
	(
	GtkWidget *widget,		// The view window.
	GdkEventSelection *event,
	gpointer data			// ->Combo_chooser.
	)
	{
//	Combo_chooser *chooser = (Combo_chooser *) data;
	cout << "SELECTION_CLEAR" << endl;
	return TRUE;
	}

/*
 *	Beginning of a drag.
 */

gint Combo_chooser::drag_begin
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	gpointer data			// ->Combo_chooser.
	)
	{
	cout << "In DRAG_BEGIN" << endl;
	Combo_chooser *chooser = (Combo_chooser *) data;
	if (chooser->selected < 0)
		return FALSE;		// ++++Display a halt bitmap.
					// Get ->combo.
	int num = chooser->info[chooser->selected].num;
	Combo *combo = chooser->combos[num];
#if 0	/* ++++++++++ */
	int w = shape->get_width(), h = shape->get_height(),
		xright = shape->get_xright(), ybelow = shape->get_ybelow();
	Image_buffer8 tbuf(w, h);	// Create buffer to render to.
	tbuf.fill8(0xff);		// Fill with 'transparent' pixel.
	unsigned char *tbits = tbuf.get_bits();
	shape->paint(&tbuf, w - 1 - xright, h - 1 - ybelow);
					// Put shape on a pixmap.
	GdkPixmap *pixmap = gdk_pixmap_new(widget->window, w, h, -1);
	gdk_draw_indexed_image(pixmap, chooser->drawgc, 0, 0, w, h,
			GDK_RGB_DITHER_NORMAL, tbits,
			tbuf.get_line_width(), chooser->palette);
	int mask_stride = (w + 7)/8;	// Round up to nearest byte.
	char *mdata = new char[mask_stride*h];
	for (int y = 0; y < h; y++)	// Do each row.
					// Do each byte.
		for (int b = 0; b < mask_stride; b++)
			{
			char bits = 0;
			unsigned char *vals = tbits + y*w + b*8;
			for (int i = 0; i < 8; i++)
				if (vals[i] != 0xff)
					bits |= (1<<i);
			mdata[y*mask_stride + b] = bits;
			}
	GdkBitmap *mask = gdk_bitmap_create_from_data(widget->window,
							mdata, w, h);
	delete mdata;
					// This will be the shape dragged.
	gtk_drag_set_icon_pixmap(context,
			gdk_window_get_colormap(widget->window), pixmap, mask,
					w - 2 - xright, h - 2 - ybelow);
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);
#endif
	return TRUE;
	}

/*
 *	Handle a scrollbar event.
 */

void Combo_chooser::scrolled
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Combo_chooser.
	)
	{
	Combo_chooser *chooser = (Combo_chooser *) data;
	gint newindex = (gint) adj->value;
	chooser->scroll(newindex);
	}

/*
 *	Callbacks for controls:
 */
#if 0
C_EXPORT void
on_insert_combo_new_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	Combo_chooser *chooser = (Combo_chooser *) user_data;
	chooser->insert(false);
}
#endif

C_EXPORT void
on_move_combo_down_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	Combo_chooser *chooser = (Combo_chooser *) user_data;
	chooser->move(false);
}
C_EXPORT void
on_move_combo_up_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	Combo_chooser *chooser = (Combo_chooser *) user_data;
	chooser->move(true);
}

/*
 *	Keystroke in draw-area.
 */
static gboolean
on_combo_key_press			(GtkEntry	*entry,
					 GdkEventKey	*event,
					 gpointer	 user_data)
{
	Combo_chooser *chooser = (Combo_chooser *) user_data;
	switch (event->keyval)
		{
	case GDK_Delete:
		chooser->remove();
		return TRUE;
#if 0
	case GDK_Insert:
		chooser->new_frame();
		return TRUE;
#endif
		}
	return FALSE;			// Let parent handle it.
}

/*
 *	Create box with 'move' controls.
 */

GtkWidget *Combo_chooser::create_controls
	(
	)
	{
	GtkWidget *frame;		// Gets each frame.
					// Create main box.
	GtkWidget *topframe = gtk_frame_new (NULL);
	gtk_widget_show(topframe);
	GtkWidget *hbox0 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox0);
	gtk_container_add (GTK_CONTAINER (topframe), hbox0);
#if 0	/* +++++Thinking about it */
	/*
	 *	The 'New/Delete' controls.
	 */
	frame = gtk_frame_new ("Edit");
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX (hbox0), frame, FALSE, FALSE, 2);
	GtkWidget *hbuttonbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), 
							GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox), 0);
	gtk_widget_show (hbuttonbox);
	gtk_container_add (GTK_CONTAINER (frame), hbuttonbox);

	GtkWidget *insert_combo_new = gtk_button_new_with_label ("New");
	gtk_widget_show (insert_combo_new);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), insert_combo_new);
	GTK_WIDGET_SET_FLAGS (insert_combo_new, GTK_CAN_DEFAULT);
	gtk_signal_connect (GTK_OBJECT (insert_combo_new), "clicked",
			GTK_SIGNAL_FUNC (on_insert_combo_new_clicked),
			this);
#endif
	/*
	 *	The 'Move' controls.
	 */
	frame = gtk_frame_new ("Move");
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX (hbox0), frame, FALSE, FALSE, 2);
	GtkWidget *bbox = gtk_hbox_new(TRUE, 0);
	gtk_widget_show (bbox);
	gtk_container_add (GTK_CONTAINER (frame), bbox);

	move_combo_down = gtk_button_new();
	gtk_widget_show (move_combo_down);
	gtk_box_pack_start (GTK_BOX (bbox), move_combo_down, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (move_combo_down, GTK_CAN_DEFAULT);
	GtkWidget *arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	gtk_widget_show(arrow);
	gtk_container_add(GTK_CONTAINER(move_combo_down), arrow);

	move_combo_up = gtk_button_new();
	gtk_widget_show (move_combo_up);
	gtk_box_pack_start (GTK_BOX (bbox), move_combo_up, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (move_combo_up, GTK_CAN_DEFAULT);
	arrow = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_OUT);
	gtk_widget_show(arrow);
	gtk_container_add(GTK_CONTAINER(move_combo_up), arrow);
	gtk_signal_connect (GTK_OBJECT (move_combo_down), "clicked",
			GTK_SIGNAL_FUNC (on_move_combo_down_clicked),
			this);
	gtk_signal_connect (GTK_OBJECT (move_combo_up), "clicked",
			GTK_SIGNAL_FUNC (on_move_combo_up_clicked),
			this);

	return topframe;
	}

/*
 *	Enable/disable controls after selection changed.
 */

void Combo_chooser::enable_controls
	(
	)
	{
	if (selected == -1)		// No selection.
		{
		if (!group)
			{
			gtk_widget_set_sensitive(move_combo_down, false);
			gtk_widget_set_sensitive(move_combo_up, false);
			}
		return;
		}
	if (!group)
		{
		gtk_widget_set_sensitive(move_combo_down, 
				info[selected].num < combos.size() - 1);
		gtk_widget_set_sensitive(move_combo_up, 
					info[selected].num > 0);
		}
	}

/*
 *	Create the list.
 */

Combo_chooser::Combo_chooser
	(
	Vga_file *i,			// Where they're kept.
	Flex_file_info *flinfo,		// Flex-file info.
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	int w, int h,			// Dimensions.
	Shape_group *g			// Filter, or null.
	) : Object_browser(g), Shape_draw(i, palbuf, gtk_drawing_area_new()),
		flex_info(flinfo), index0(0),
		info(0), info_cnt(0), sel_changed(0)
	{
	int num_combos = flinfo->size();
					// We need 'shapes.vga'.
	Shape_file_info *svga_info = 
				ExultStudio::get_instance()->get_vgafile();
	Shapes_vga_file *svga = svga_info ?
			(Shapes_vga_file *) svga_info->get_ifile() : 0;
	combos.resize(num_combos);	// Set size of list.
	if (!svga)
		num_combos = 0;
					// Read them all in.
	for (int i = 0; i < num_combos; i++)
		{
		size_t len;
		unsigned char *buf = (unsigned char *) flex_info->get(i, len);
		Combo *combo = new Combo(svga);
		combo->read(buf, len);
		combos[i] = combo;	// Store in list.
		}
					// Put things in a vert. box.
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	set_widget(vbox); // This is our "widget"
	gtk_widget_show(vbox);
	
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	
					// A frame looks nice.
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
					// NOTE:  draw is in Shape_draw.
					// Indicate the events we want.
	gtk_widget_set_events(draw, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK
		| GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON1_MOTION_MASK | GDK_KEY_PRESS_MASK);
					// Set "configure" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "configure_event",
				GTK_SIGNAL_FUNC(configure), this);
					// Set "expose" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
				GTK_SIGNAL_FUNC(expose), this);
					// Keystroke.
	gtk_signal_connect(GTK_OBJECT(draw), "key-press-event",
		      GTK_SIGNAL_FUNC (on_combo_key_press),
		      this);
	GTK_WIDGET_SET_FLAGS(draw, GTK_CAN_FOCUS);
					// Set mouse click handler.
	gtk_signal_connect(GTK_OBJECT(draw), "button_press_event",
				GTK_SIGNAL_FUNC(mouse_press), this);
					// Mouse motion.
	gtk_signal_connect(GTK_OBJECT(draw), "drag_begin",
				GTK_SIGNAL_FUNC(drag_begin), this);
#ifdef WIN32
// required to override GTK+ Drag and Drop
	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
				GTK_SIGNAL_FUNC(win32_drag_motion), this);
#endif
//	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
//				GTK_SIGNAL_FUNC(Mouse_drag_motion), this);
	gtk_signal_connect (GTK_OBJECT(draw), "drag_data_get",
				GTK_SIGNAL_FUNC(drag_data_get), this);
	gtk_signal_connect (GTK_OBJECT(draw), "selection_clear_event",
				GTK_SIGNAL_FUNC(selection_clear), this);
	gtk_container_add (GTK_CONTAINER (frame), draw);
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), w, h);
	gtk_widget_show(draw);
					// Want a scrollbar for the combos.
	GtkObject *combo_adj = gtk_adjustment_new(0, 0, 
				num_combos, 1, 
				4, 1.0);
	combo_scroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(combo_adj));
					// Update window when it stops.
	gtk_range_set_update_policy(GTK_RANGE(combo_scroll),
					GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX(hbox), combo_scroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(combo_adj), "value_changed",
					GTK_SIGNAL_FUNC(scrolled), this);
	gtk_widget_show(combo_scroll);
					// At the bottom, status bar:
	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
	gtk_widget_show(hbox1);
					// At left, a status bar.
	sbar = gtk_statusbar_new();
	sbar_sel = gtk_statusbar_get_context_id(GTK_STATUSBAR(sbar),
							"selection");
	gtk_box_pack_start(GTK_BOX(hbox1), sbar, TRUE, TRUE, 0);
	gtk_widget_show(sbar);
					// Add controls to bottom.
	gtk_box_pack_start(GTK_BOX(vbox), create_controls(), FALSE, FALSE, 0);
	}

/*
 *	Delete.
 */

Combo_chooser::~Combo_chooser
	(
	)
	{
	gtk_widget_destroy(get_widget());
	delete [] info;
	int i;
	int cnt = combos.size();
	for (i = 0; i < cnt; i++)	// Delete all the combos.
		delete combos[i];
	}

/*
 *	Add a new or updated combo.
 *
 *	Output:	Index of entry.
 */

int Combo_chooser::add
	(
	Combo *newcombo,		// We'll own this.
	int index			// Index to replace, or -1 to add new.
	)
	{
	if (index == -1)
		{			// New.
		combos.push_back(newcombo);
		index = combos.size() - 1;	// Index of new entry.
		}
	else
		{
		assert(index >= 0 && index < combos.size());
		delete combos[index];
		combos[index] = newcombo;
		}
	GtkAdjustment *adj = 
			gtk_range_get_adjustment(GTK_RANGE(combo_scroll));
	adj->upper = combos.size();
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	render();
	show();
	return index;			// Return index.
	}

/*
 *	Remove selected entry.
 */

void Combo_chooser::remove
	(
	)
	{
	if (selected < 0)
		return;
	int tnum = info[selected].num;
	Combo_editor *combowin = ExultStudio::get_instance()->get_combowin();
	if (combowin && combowin->is_visible() && combowin->file_index == tnum)
		{
		EStudio::Alert("Can't remove the combo you're editing");
		return;
		}
	if (EStudio::Prompt("Okay to remove selected combo?", "Yes", "no")
								!= 0)
		return;
	selected = -1;
	Combo *todel = combos[tnum];
	delete todel;			// Delete from our list.
	combos.erase(combos.begin() + tnum);
	flex_info->set_modified();
	flex_info->remove(tnum);	// Update flex-file list.
	GtkAdjustment *adj = 		// Update scrollbar.
			gtk_range_get_adjustment(GTK_RANGE(combo_scroll));
	adj->upper = combos.size();
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	render();
	show();
	}

/*
 *	Bring up editor for selected combo.
 */

void Combo_chooser::edit
	(
	)
	{
	if (selected < 0)
		return;
	Combo_editor *combowin = ExultStudio::get_instance()->get_combowin();
	if (combowin && combowin->is_visible())
		{
		EStudio::Alert("You're already editing a combo");
		return;
		}
	int tnum = info[selected].num;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_combo_window();	// Open it.
	Combo_editor *ed = studio->get_combowin();
	if (!ed || !ed->is_visible())
		return;			// Failed.  Shouldn't happen.
	ed->set_combo(new Combo(*(combos[tnum])), tnum);
	}

/*
 *	Configure the viewing window.
 */

gint Combo_chooser::configure
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Combo_chooser
	)
	{
	Combo_chooser *chooser = (Combo_chooser *) data;
	chooser->Shape_draw::configure(widget);
	chooser->render();
					// Set new scroll amounts.
	int w = event->width, h = event->height;
	int per_row = (w - border)/(128 + border);
	int num_rows = (h - border)/(128 + border);
	int page_size = per_row*num_rows;
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(
						chooser->combo_scroll));
	adj->step_increment = per_row;
	adj->page_increment = page_size;
	adj->page_size = page_size;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
#if 0	/* ++++++Later */
	if (chooser->group)		// Filtering?
		chooser->enable_drop();	// Can drop combos here.
#endif
	return (TRUE);
	}

/*
 *	Handle an expose event.
 */

gint Combo_chooser::expose
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Combo_chooser.
	)
	{
	Combo_chooser *chooser = (Combo_chooser *) data;
	chooser->show(event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Handle a mouse button press event.
 */

gint Combo_chooser::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Combo_chooser.
	)
	{
	gtk_widget_grab_focus(widget);	// Enables keystrokes.
	Combo_chooser *chooser = (Combo_chooser *) data;
	int old_selected = chooser->selected;
					// Search through entries.
	for (int i = 0; i < chooser->info_cnt; i++)
		if (chooser->info[i].box.has_point(
					(int) event->x, (int) event->y))
			{		// Found the box?
					// Indicate we can drag.
#ifdef WIN32
// Here, we have to override GTK+'s Drag and Drop, which is non-OLE and
// usually stucks outside the program window. I think it's because
// the dragged shape only receives mouse motion events when the new mouse pointer
// position is *still* inside the shape. So if you move the mouse too fast,
// we are stuck.
			win32_button = true;
#else
			GtkTargetEntry tents[1];
			tents[0].target = U7_TARGET_COMBOID_NAME;
			tents[0].flags = 0;
			tents[0].info = U7_TARGET_COMBOID;
			gtk_drag_source_set (chooser->draw, 
				GDK_BUTTON1_MASK, tents, 1,
			   (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE));
#endif
			chooser->selected = i;
			chooser->render();
			chooser->show();
					// Tell client.
			if (chooser->sel_changed)
				(*chooser->sel_changed)();
			break;
			}
	if (chooser->selected == old_selected && old_selected >= 0)
		{			// Same square.  Check for dbl-click.
		if (((GdkEvent *) event)->type == GDK_2BUTTON_PRESS)
			chooser->edit();
		}
#if 0
	if (event->button == 3 && chooser->selected >= 0)
		{
					// Clean out old.
		if (chooser->popup)
			gtk_widget_destroy(chooser->popup);
		GtkWidget *popup = Create_browser_popup(chooser);
		chooser->popup = popup;
		gtk_menu_popup(GTK_MENU(popup), 0, 0, 0, 0, event->button,
							event->time);
		}
#endif
	return (TRUE);
	}


/*
 *	Move currently-selected combo up or down.
 */

void Combo_chooser::move
	(
	bool upwards
	)
	{
	if (selected < 0)
		return;			// Shouldn't happen.
	int tnum = info[selected].num;
	if ((tnum == 0 && upwards) || (tnum == combos.size() - 1 && !upwards))
		return;
	if (upwards)			// Going to swap tnum & tnum+1.
		tnum--;
	Combo *tmp = combos[tnum];
	combos[tnum] = combos[tnum + 1];
	combos[tnum + 1] = tmp;
	selected += upwards ? -1 : 1;
					// Update editor if open.
	Combo_editor *combowin = ExultStudio::get_instance()->get_combowin();
	if (combowin && combowin->is_visible())
		{
		if (combowin->file_index == tnum)
			combowin->file_index = tnum + 1;
		else if (combowin->file_index == tnum + 1)
			combowin->file_index = tnum;
		}
	flex_info->set_modified();
	flex_info->swap(tnum);		// Update flex-file list.
	render();
	show();
	}

