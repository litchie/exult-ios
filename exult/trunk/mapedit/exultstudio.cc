/**
 **	A GIMP Extension for U7 Data files
 **/

/*
Copyright (C) 2001 - The Exult Team

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

#include "../alpha_kludges.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include "shapelst.h"
#include "paledit.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"

Vga_file *ifile = 0;
char **names = 0;
GtkWidget *topwin = 0;
Shape_chooser *chooser = 0;
Palette_edit *paled = 0;

void
on_shapes_add_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_shapes_remove_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_add_palette_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_palette_remove_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{

}

GtkWidget*
create_exult_studio (void)
{
  GtkWidget *window1;
  GtkWidget *vbox1;
  GtkWidget *frame1;
  GtkWidget *hbox1;
  GtkWidget *vbox3;
  GtkWidget *list1;
  GtkWidget *hbox2;
  GtkWidget *button2;
  GtkWidget *button3;
  GtkWidget *vseparator1;
  GtkWidget *vbox2;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *button1;
  GtkWidget *frame2;
  GtkWidget *hbox3;
  GtkWidget *vbox4;
  GtkWidget *list2;
  GtkWidget *hbox4;
  GtkWidget *button4;
  GtkWidget *button5;
  GtkWidget *vseparator2;
  GtkWidget *vbox5;
  GtkWidget *label3;
  GtkWidget *label4;

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  gtk_window_set_title (GTK_WINDOW (window1), "Exult Shape Studio");

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);

  frame1 = gtk_frame_new ("Shapes");
  gtk_widget_ref (frame1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "frame1", frame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (frame1), hbox1);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  list1 = gtk_list_new ();
  gtk_widget_ref (list1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "list1", list1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (list1);
  gtk_box_pack_start (GTK_BOX (vbox3), list1, TRUE, TRUE, 0);

  hbox2 = gtk_hbox_new (TRUE, 0);
  gtk_widget_ref (hbox2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hbox2", hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, FALSE, TRUE, 0);

  button2 = gtk_button_new_with_label ("Add...");
  gtk_widget_ref (button2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button2", button2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button2);
  gtk_box_pack_start (GTK_BOX (hbox2), button2, TRUE, TRUE, 0);

  button3 = gtk_button_new_with_label ("Remove");
  gtk_widget_ref (button3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button3", button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button3);
  gtk_box_pack_start (GTK_BOX (hbox2), button3, TRUE, TRUE, 0);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_ref (vseparator1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vseparator1", vseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vseparator1);
  gtk_box_pack_start (GTK_BOX (hbox1), vseparator1, FALSE, TRUE, 4);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  label1 = gtk_label_new ("Shape browser");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox2), label1, FALSE, FALSE, 0);
  
  /*
  label2 = gtk_label_new ("Shape Browser Widget Placeholder");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox2), label2, TRUE, TRUE, 0);
  */
  chooser = new Shape_chooser(ifile, names, vbox2, 400, 64);

  button1 = gtk_button_new_with_label ("Load in GIMP");
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);
  gtk_box_pack_start (GTK_BOX (vbox2), button1, FALSE, FALSE, 0);

  frame2 = gtk_frame_new ("Palettes");
  gtk_widget_ref (frame2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "frame2", frame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hbox3", hbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (frame2), hbox3);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox4", vbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox4);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox4, TRUE, TRUE, 0);

  list2 = gtk_list_new ();
  gtk_widget_ref (list2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "list2", list2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (list2);
  gtk_box_pack_start (GTK_BOX (vbox4), list2, TRUE, TRUE, 0);

  hbox4 = gtk_hbox_new (TRUE, 0);
  gtk_widget_ref (hbox4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hbox4", hbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox4, FALSE, TRUE, 0);

  button4 = gtk_button_new_with_label ("Add...");
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button4);
  gtk_box_pack_start (GTK_BOX (hbox4), button4, TRUE, TRUE, 0);

  button5 = gtk_button_new_with_label ("Remove");
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox4), button5, TRUE, TRUE, 0);

  vseparator2 = gtk_vseparator_new ();
  gtk_widget_ref (vseparator2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vseparator2", vseparator2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vseparator2);
  gtk_box_pack_start (GTK_BOX (hbox3), vseparator2, FALSE, TRUE, 4);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox5, TRUE, TRUE, 0);

  label3 = gtk_label_new ("Palette Browser");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (vbox5), label3, FALSE, FALSE, 0);

  /*
  label4 = gtk_label_new ("Palette Browser Widget Placeholder");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (vbox5), label4, TRUE, TRUE, 0);
  */
  U7object pal("static/palettes.flx", 0);
  size_t len;
  unsigned char *buf;		// this may throw an exception
  buf = (unsigned char *) pal.retrieve(len);
  guint32 colors[256];
  for (int i = 0; i < 256; i++)
    colors[i] = (buf[3*i]<<16)*4 + (buf[3*i+1]<<8)*4 + 
 		buf[3*i+2]*4;
  paled = new Palette_edit(colors, vbox5, 128, 128);

  gtk_signal_connect (GTK_OBJECT (button2), "clicked",
                      GTK_SIGNAL_FUNC (on_shapes_add_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button3), "clicked",
                      GTK_SIGNAL_FUNC (on_shapes_remove_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
                      GTK_SIGNAL_FUNC (on_add_palette_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
                      GTK_SIGNAL_FUNC (on_palette_remove_clicked),
                      NULL);

  return window1;
}

GtkWidget*
create_fileselection1 (void)
{
  GtkWidget *fileselection1;
  GtkWidget *ok_button1;
  GtkWidget *cancel_button1;

  fileselection1 = gtk_file_selection_new ("Select shape file");
  gtk_object_set_data (GTK_OBJECT (fileselection1), "fileselection1", fileselection1);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection1), 10);

  ok_button1 = GTK_FILE_SELECTION (fileselection1)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection1), "ok_button1", ok_button1);
  gtk_widget_show (ok_button1);
  GTK_WIDGET_SET_FLAGS (ok_button1, GTK_CAN_DEFAULT);

  cancel_button1 = GTK_FILE_SELECTION (fileselection1)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection1), "cancel_button1", cancel_button1);
  gtk_widget_show (cancel_button1);
  GTK_WIDGET_SET_FLAGS (cancel_button1, GTK_CAN_DEFAULT);

  return fileselection1;
}

GtkWidget*
create_fileselection2 (void)
{
  GtkWidget *fileselection2;
  GtkWidget *ok_button2;
  GtkWidget *cancel_button2;

  fileselection2 = gtk_file_selection_new ("Select Palette File");
  gtk_object_set_data (GTK_OBJECT (fileselection2), "fileselection2", fileselection2);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection2), 10);

  ok_button2 = GTK_FILE_SELECTION (fileselection2)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection2), "ok_button2", ok_button2);
  gtk_widget_show (ok_button2);
  GTK_WIDGET_SET_FLAGS (ok_button2, GTK_CAN_DEFAULT);

  cancel_button2 = GTK_FILE_SELECTION (fileselection2)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection2), "cancel_button2", cancel_button2);
  gtk_widget_show (cancel_button2);
  GTK_WIDGET_SET_FLAGS (cancel_button2, GTK_CAN_DEFAULT);

  return fileselection2;
}



/*
 *	Quit.
 */

int Quit
	(
	)
	{
	delete chooser;
	int num_shapes = ifile->get_num_shapes();
	delete ifile;
	for (int i = 0; i < num_shapes; i++)
		delete names[i];
	delete [] names;
	gtk_exit(0);
	return (FALSE);			// Shouldn't get here.
	}

/*
 *	Main program.
 */

int main
	(
	int argc,
	char **argv
	)
	{
					// Open file.
	ifile = new Vga_file("static/shapes.vga");
	if (!ifile->is_good())
		{
		cerr << "Error opening image file 'shapes.vga'.\n";
		return (1);
		}
					// Read in shape names.
	int num_names = ifile->get_num_shapes();
	names = new char *[num_names];
	Flex *items = new Flex("static/text.flx");
	size_t len;
	for (int i = 0; i < num_names; i++)
		names[i] = items->retrieve(i, len);
	delete items;
	gtk_init(&argc, &argv);
	gdk_rgb_init();
					// Create top-level window.
	topwin = create_exult_studio ();
	gtk_signal_connect(GTK_OBJECT(topwin), "delete_event",
				GTK_SIGNAL_FUNC(Quit), NULL);
					// Set border width of top window.

	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topwin), vbox);
	gtk_widget_show(vbox);
	
	gtk_widget_show(topwin);	// Show top window.
	gtk_main();
	return (0);
	}
