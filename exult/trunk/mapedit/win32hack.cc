extern "C" void on_open_egg_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	);
extern "C" void on_egg_apply_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" void on_egg_cancel_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" gboolean on_egg_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	);
extern "C" gboolean on_egg_monster_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	);
extern "C" gboolean on_monst_shape_focus_out_event
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	);
extern "C" void on_open_npc_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	);
extern "C" void on_npc_apply_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" void on_npc_cancel_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" gboolean on_npc_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	);
extern "C" gboolean on_npc_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	);
extern "C" gboolean on_npc_shape_focus_out_event
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	);
extern "C" gboolean on_npc_face_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	);
extern "C" void on_npc_set_sched
	(
	GtkWidget *btn,			// One of the 'set' buttons.
	gpointer user_data
	);
extern "C" void on_obj_apply_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" void on_obj_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" gboolean on_obj_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	);
extern "C" gboolean on_obj_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	);
extern "C" gboolean on_obj_shape_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	);
extern "C" gboolean on_obj_pos_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	);
extern "C" void on_shinfo_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	);
extern "C" gboolean on_shape_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	);
extern "C" gboolean on_shinfo_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	);
