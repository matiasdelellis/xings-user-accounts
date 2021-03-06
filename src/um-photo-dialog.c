/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright 2009-2010  Red Hat, Inc,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Written by: Matthias Clasen <mclasen@redhat.com>
 */

#include "config.h"

#include <stdlib.h>
#include <math.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#define GDK_PIXBUF_ENABLE_BACKEND
#include <gtk/gtk.h>
#include <act/act.h>

#ifdef HAVE_CHEESE
#include <cheese-avatar-chooser.h>
#include <cheese-camera-device.h>
#include <cheese-camera-device-monitor.h>
#endif /* HAVE_CHEESE */

#include "um-photo-dialog.h"
#include "cc-crop-area.h"
#include "um-utils.h"

#include "xings-user-accounts-common.h"

#define DEFAULT_IMAGE_SIZE 512
#define ROW_SPAN 6

#define PREVIEW_IMAGE_WIDTH 96

struct _UmPhotoDialog {
	GtkWidget *photo_popup;

	GtkWidget *popup_button;
	GtkWidget *crop_area;

#ifdef HAVE_CHEESE
	CheeseCameraDeviceMonitor *monitor;
	GtkWidget *take_photo_menuitem;
	guint num_cameras;
#endif /* HAVE_CHEESE */

	ActUser *user;
};

static void
crop_dialog_response (GtkWidget     *dialog,
                      gint           response_id,
                      UmPhotoDialog *um)
{
	GdkPixbuf *pb, *pb2;

	if (response_id != GTK_RESPONSE_ACCEPT) {
		um->crop_area = NULL;
		gtk_widget_destroy (dialog);
		return;
	}

	pb = cc_crop_area_get_picture (CC_CROP_AREA (um->crop_area));
	pb2 = gdk_pixbuf_scale_simple (pb,
	                               DEFAULT_IMAGE_SIZE,
	                               DEFAULT_IMAGE_SIZE,
	                               GDK_INTERP_BILINEAR);

	set_user_icon_data (um->user, pb2);

	g_object_unref (pb2);
	g_object_unref (pb);

	um->crop_area = NULL;
	gtk_widget_destroy (dialog);
}

static void
um_photo_dialog_crop (UmPhotoDialog *um,
                      GdkPixbuf     *pixbuf)
{
	GtkWidget *dialog;

	dialog = gtk_dialog_new_with_buttons ("",
	                                      GTK_WINDOW (gtk_widget_get_toplevel (um->popup_button)),
	                                      0,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("Select"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	gtk_window_set_icon_name (GTK_WINDOW (dialog), "system-users");

	g_signal_connect (G_OBJECT (dialog), "response",
		G_CALLBACK (crop_dialog_response), um);

        /* Content */
	um->crop_area = cc_crop_area_new ();
	cc_crop_area_set_min_size (CC_CROP_AREA (um->crop_area), 48, 48);
	cc_crop_area_set_constrain_aspect (CC_CROP_AREA (um->crop_area), TRUE);
	cc_crop_area_set_picture (CC_CROP_AREA (um->crop_area), pixbuf);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
	                    um->crop_area,
	                    TRUE, TRUE, 8);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 400, 300);

	gtk_widget_show_all (dialog);
}

static void
file_chooser_response (GtkDialog     *chooser,
                       gint           response,
                       UmPhotoDialog *um)
{
	gchar *filename;
	GError *error;
	GdkPixbuf *pixbuf;

	if (response != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy (GTK_WIDGET (chooser));
		return;
	}

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

	error = NULL;
	pixbuf = gdk_pixbuf_new_from_file (filename, &error);
	if (pixbuf == NULL) {
		g_warning ("Failed to load %s: %s", filename, error->message);
		g_error_free (error);
	}
	g_free (filename);

	gtk_widget_destroy (GTK_WIDGET (chooser));

	um_photo_dialog_crop (um, pixbuf);
	g_object_unref (pixbuf);
}

static void
update_preview (GtkFileChooser *icon_chooser,
                GtkWidget      *preview_widget)
{
	GdkPixbuf *scaled_pixbuf = NULL, *pixbuf = NULL;
	gchar *filename = NULL;
	double scale;

	filename = gtk_file_chooser_get_filename (icon_chooser);
	if (filename != NULL) {
		pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
	}

	if (pixbuf != NULL) {
		if (gdk_pixbuf_get_width (pixbuf) > PREVIEW_IMAGE_WIDTH) {
			scale = (double) gdk_pixbuf_get_height (pixbuf) /
			                 gdk_pixbuf_get_width (pixbuf);

			scaled_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
			                                         PREVIEW_IMAGE_WIDTH,
			                                         scale * PREVIEW_IMAGE_WIDTH,
			                                         GDK_INTERP_BILINEAR);

			g_object_unref (pixbuf);
			pixbuf = scaled_pixbuf;
		}
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview_widget), pixbuf);
	}
	else {
		gtk_image_set_from_icon_name (GTK_IMAGE (preview_widget),
		                              "dialog-question",
		                              GTK_ICON_SIZE_DIALOG);
	}
}

static void
um_photo_dialog_select_file (UmPhotoDialog *um)
{
	GtkWidget *chooser;
	const gchar *folder;
	GtkWidget *preview;
	GtkFileFilter *filter;

	chooser = gtk_file_chooser_dialog_new (_("Browse for more pictures"),
	                                       GTK_WINDOW (gtk_widget_get_toplevel (um->popup_button)),
	                                       GTK_FILE_CHOOSER_ACTION_OPEN,
	                                       _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                       _("_Open"), GTK_RESPONSE_ACCEPT,
	                                       NULL);

	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);

	preview = gtk_image_new ();
	gtk_widget_set_size_request (preview, 128, -1);
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (chooser), preview);
	gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (chooser), FALSE);
	gtk_widget_show (preview);

	/* Preview has to be generated after default handler of "selection-changed"
	 * signal, otherwise dialog response sensitivity is rewritten (Bug 547988).
	 * Preview also has to be generated on "selection-changed" signal to reflect
	 * all changes (Bug 660877). */
	g_signal_connect_after (chooser, "selection-changed",
	                        G_CALLBACK (update_preview), preview);

	folder = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);
	if (folder)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser),
			folder);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pixbuf_formats (filter);
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (chooser, "response",
		G_CALLBACK (file_chooser_response), um);

	gtk_window_present (GTK_WINDOW (chooser));
}

static void
none_icon_selected (GtkMenuItem   *menuitem,
                    UmPhotoDialog *um)
{
	act_user_set_icon_file (um->user, "");
}

static void
file_icon_selected (GtkMenuItem   *menuitem,
                    UmPhotoDialog *um)
{
	um_photo_dialog_select_file (um);
}

#ifdef HAVE_CHEESE
static gboolean
destroy_chooser (GtkWidget *chooser)
{
	gtk_widget_destroy (chooser);
	return FALSE;
}

static void
webcam_response_cb (GtkDialog     *dialog,
                    int            response,
                    UmPhotoDialog  *um)
{
	if (response == GTK_RESPONSE_ACCEPT) {
		GdkPixbuf *pb, *pb2;

		g_object_get (G_OBJECT (dialog), "pixbuf", &pb, NULL);
		pb2 = gdk_pixbuf_scale_simple (pb,
		                               DEFAULT_IMAGE_SIZE,
		                               DEFAULT_IMAGE_SIZE,
		                               GDK_INTERP_BILINEAR);

		set_user_icon_data (um->user, pb2);

		g_object_unref (pb2);
		g_object_unref (pb);
	}

	if (response != GTK_RESPONSE_DELETE_EVENT &&
		response != GTK_RESPONSE_NONE)
		g_idle_add ((GSourceFunc) destroy_chooser, dialog);
}

static void
webcam_icon_selected (GtkMenuItem   *menuitem,
                      UmPhotoDialog *um)
{
	GtkWidget *window;

	window = cheese_avatar_chooser_new ();
	gtk_window_set_transient_for (GTK_WINDOW (window),
		GTK_WINDOW (gtk_widget_get_toplevel (um->popup_button)));
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);
	g_signal_connect (G_OBJECT (window), "response",
		G_CALLBACK (webcam_response_cb), um);
	gtk_widget_show (window);
}

static void
update_photo_menu_status (UmPhotoDialog *um)
{
	if (um->num_cameras == 0)
		gtk_widget_set_sensitive (um->take_photo_menuitem, FALSE);
	else
		gtk_widget_set_sensitive (um->take_photo_menuitem, TRUE);
}

static void
device_added (CheeseCameraDeviceMonitor *monitor,
              CheeseCameraDevice        *device,
              UmPhotoDialog             *um)
{
	um->num_cameras++;
	update_photo_menu_status (um);
}

static void
device_removed (CheeseCameraDeviceMonitor *monitor,
                const char                *id,
                UmPhotoDialog             *um)
{
	um->num_cameras--;
	update_photo_menu_status (um);
}
#endif /* HAVE_CHEESE */

static void
stock_icon_selected (GtkMenuItem   *menuitem,
                     UmPhotoDialog *um)
{
	const char *filename;

	filename = g_object_get_data (G_OBJECT (menuitem), "filename");
	act_user_set_icon_file (um->user, filename);
}

static GtkWidget *
menu_item_for_filename (UmPhotoDialog *um,
                        const char    *filename)
{
	GtkWidget *image, *menuitem;
	GFile *file;
	GIcon *icon;

	file = g_file_new_for_path (filename);
	icon = g_file_icon_new (file);
	g_object_unref (file);
	image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_DIALOG);
	g_object_unref (icon);

	menuitem = gtk_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menuitem), image);
	gtk_widget_show_all (menuitem);

	g_object_set_data_full (G_OBJECT (menuitem), "filename",
		g_strdup (filename), (GDestroyNotify) g_free);
	g_signal_connect (G_OBJECT (menuitem), "activate",
		G_CALLBACK (stock_icon_selected), um);

	return menuitem;
}

static GStrv
get_settings_facesdirs (void)
{
	GSettings *settings;
	GStrv settings_dirs;
	GPtrArray *facesdirs;
	gchar *path = NULL;
	gint i = 0;

	settings = g_settings_new (XUA_SETTINGS_SCHEMA);
	settings_dirs = g_settings_get_strv (settings, XUA_SETTINGS_KEY_AVATAR_DIRECTORIES);
	facesdirs = g_ptr_array_new ();

	if (settings_dirs != NULL) {
		for (i = 0; settings_dirs[i] != NULL; i++) {
			path = settings_dirs[i];
			if (g_strcmp0 (path, "") != 0)
				g_ptr_array_add (facesdirs, g_strdup (path));
		}
	}

	g_ptr_array_add (facesdirs, NULL);

	g_strfreev (settings_dirs);
	g_object_unref (settings);

	return (GStrv) g_ptr_array_free (facesdirs, FALSE);
}

static GStrv
get_system_facesdirs (void)
{
	const char * const * data_dirs;
	GPtrArray *facesdirs;
	gchar *path = NULL;
	int i = 0;

	facesdirs = g_ptr_array_new ();

	data_dirs = g_get_system_data_dirs ();
	for (i = 0; data_dirs[i] != NULL; i++) {
		path = g_build_filename (data_dirs[i], "pixmaps", "xings", "faces", NULL);
		g_ptr_array_add (facesdirs, path);
	}

	g_ptr_array_add (facesdirs, NULL);

	return (GStrv) g_ptr_array_free (facesdirs, FALSE);
}

static gboolean
add_faces_from_dirs (UmPhotoDialog *um,
                     GStrv          facesdirs,
                     gboolean       add_all,
                     guint         *x,
                     guint         *y)
{
	GtkWidget *menuitem;
	GDir *dir;
	gboolean added_faces = FALSE;
	const gchar *face;
	gchar *filename;
	guint i = 0;

	for (i = 0; facesdirs[i] != NULL; i++) {
		dir = g_dir_open (facesdirs[i], 0, NULL);
		if (dir == NULL)
			continue;

		while ((face = g_dir_read_name (dir)) != NULL) {
			filename = g_build_filename (facesdirs[i], face, NULL);
			menuitem = menu_item_for_filename (um, filename);
			g_free (filename);

			if (menuitem == NULL)
				continue;

			gtk_menu_attach (GTK_MENU (um->photo_popup),
			                 GTK_WIDGET (menuitem),
			                 *x, *x + 1,
			                 *y, *y + 1);
			gtk_widget_show (menuitem);

			(*x)++;
			if (*x >= ROW_SPAN - 1) {
				(*y)++;
				*x = 0;
			}

			added_faces = TRUE;
		}
		g_dir_close (dir);

		if (added_faces && !add_all)
			break;
	}

	return added_faces;
}

static void
setup_photo_popup (UmPhotoDialog *um)
{
	GtkWidget *menu, *menuitem, *image;
	GStrv settings_facesdirs = NULL;
	GStrv system_facesdirs = NULL;
	guint x = 0, y = 0;
	gboolean none_item_shown = FALSE;
	gboolean added_faces;

	menu = gtk_menu_new ();
	um->photo_popup = menu;

	/* Add avatar from settings or default folders*/
	settings_facesdirs = get_settings_facesdirs ();
	added_faces = add_faces_from_dirs (um, settings_facesdirs, TRUE, &x, &y);
	g_strfreev (settings_facesdirs);

	if (!added_faces) {
		system_facesdirs = get_system_facesdirs ();
		added_faces = add_faces_from_dirs (um, system_facesdirs, FALSE, &x, &y);
		g_strfreev (system_facesdirs);
	}

	if (!added_faces)
		goto skip_faces;

	/* Default avatar. */
	image = gtk_image_new_from_icon_name ("avatar-default", GTK_ICON_SIZE_DIALOG);
	menuitem = gtk_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menuitem), image);
	gtk_widget_show_all (menuitem);
	gtk_menu_attach (GTK_MENU (menu),
	                 GTK_WIDGET (menuitem),
	                 x, x + 1,
	                 y, y + 1);
	g_signal_connect (G_OBJECT (menuitem), "activate",
	                  G_CALLBACK (none_icon_selected), um);
	gtk_widget_show (menuitem);
	none_item_shown = TRUE;
	y++;

skip_faces:
	if (!none_item_shown) {
		menuitem = gtk_menu_item_new_with_label (_("Disable image"));
		gtk_menu_attach (GTK_MENU (menu),
		                 GTK_WIDGET (menuitem),
		                 0, ROW_SPAN - 1,
		                 y, y + 1);
		g_signal_connect (G_OBJECT (menuitem), "activate",
		                  G_CALLBACK (none_icon_selected), um);
		gtk_widget_show (menuitem);
		y++;
	}

	/* Separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_attach (GTK_MENU (menu),
	                 GTK_WIDGET (menuitem),
	                 0, ROW_SPAN - 1,
	                 y, y + 1);
	gtk_widget_show (menuitem);

	y++;

#ifdef HAVE_CHEESE
	um->take_photo_menuitem = gtk_menu_item_new_with_label (_("Take a photo???"));
	gtk_menu_attach (GTK_MENU (menu),
	                 GTK_WIDGET (um->take_photo_menuitem),
	                 0, ROW_SPAN - 1,
	                 y, y + 1);
	g_signal_connect (G_OBJECT (um->take_photo_menuitem), "activate",
		G_CALLBACK (webcam_icon_selected), um);
	gtk_widget_set_sensitive (um->take_photo_menuitem, FALSE);
	gtk_widget_show (um->take_photo_menuitem);

	um->monitor = cheese_camera_device_monitor_new ();
	g_signal_connect (G_OBJECT (um->monitor), "added",
		G_CALLBACK (device_added), um);
	g_signal_connect (G_OBJECT (um->monitor), "removed",
		G_CALLBACK (device_removed), um);
	cheese_camera_device_monitor_coldplug (um->monitor);

	update_photo_menu_status (um);

	y++;
#endif /* HAVE_CHEESE */

	menuitem = gtk_menu_item_new_with_label (_("Browse for more pictures???"));
	gtk_menu_attach (GTK_MENU (menu),
	                 GTK_WIDGET (menuitem),
	                 0, ROW_SPAN - 1,
	                 y, y + 1);
	g_signal_connect (G_OBJECT (menuitem), "activate",
	                  G_CALLBACK (file_icon_selected), um);
	gtk_widget_show (menuitem);
}

static void
popup_icon_menu (GtkToggleButton *button, UmPhotoDialog *um)
{
	GtkWindow *window;

	window = GTK_WINDOW (gtk_widget_get_toplevel (um->photo_popup));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) && !gtk_widget_get_visible (um->photo_popup)) {
		gtk_menu_popup_at_widget (GTK_MENU (um->photo_popup),
		                          GTK_WIDGET (button),
		                          GDK_GRAVITY_NORTH_WEST,
		                          GDK_GRAVITY_SOUTH_WEST,
		                          NULL);

		gtk_window_set_transient_for (window, GTK_WINDOW (gtk_widget_get_toplevel (um->popup_button)));
	} else {
		gtk_menu_popdown (GTK_MENU (um->photo_popup));

		gtk_window_set_transient_for (window, NULL);
	}
}

static gboolean
on_popup_button_button_pressed (GtkToggleButton *button,
                                GdkEventButton  *event,
                                UmPhotoDialog   *um)
{
	if (event->button == 1) {
		if (!gtk_widget_get_visible (um->photo_popup)) {
			popup_icon_menu (button, um);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
		} else {
			gtk_menu_popdown (GTK_MENU (um->photo_popup));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

static void
on_photo_popup_unmap (GtkWidget     *popup_menu,
                      UmPhotoDialog *um)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (um->popup_button), FALSE);
}

static void
popup_button_draw (GtkWidget      *widget,
                   cairo_t        *cr,
                   UmPhotoDialog  *um)
{
	if (!(gtk_widget_get_state_flags (gtk_bin_get_child (GTK_BIN (widget))) & GTK_STATE_FLAG_PRELIGHT) &&
		!gtk_widget_is_focus (widget)) {
		return;
	}

	down_arrow (gtk_widget_get_style_context (widget),
	            cr,
	            gtk_widget_get_allocated_width (widget) - 12,
	            gtk_widget_get_allocated_height (widget) - 12,
	            12, 12);
}

static void
popup_button_focus_changed (GObject       *button,
                            GParamSpec    *pspec,
                            UmPhotoDialog *um)
{
	gtk_widget_queue_draw (gtk_bin_get_child (GTK_BIN (button)));
}

UmPhotoDialog *
um_photo_dialog_new (GtkWidget *button)
{
	UmPhotoDialog *um;

	um = g_new0 (UmPhotoDialog, 1);

	/* Set up the popup */
	um->popup_button = button;

	setup_photo_popup (um);

	g_signal_connect (button, "toggled",
		G_CALLBACK (popup_icon_menu), um);
	g_signal_connect (button, "button-press-event",
		G_CALLBACK (on_popup_button_button_pressed), um);
	g_signal_connect (button, "notify::is-focus",
		G_CALLBACK (popup_button_focus_changed), um);
	g_signal_connect_after (button, "draw",
		G_CALLBACK (popup_button_draw), um);

	g_signal_connect (um->photo_popup, "unmap",
		G_CALLBACK (on_photo_popup_unmap), um);

	return um;
}

void
um_photo_dialog_free (UmPhotoDialog *um)
{
	gtk_widget_destroy (um->photo_popup);

#ifdef HAVE_CHEESE
	if (um->monitor)
		g_object_unref (um->monitor);
#endif
	if (um->user)
		g_object_unref (um->user);

	g_free (um);
}

static void
clear_tip (GtkMenuItem  *item,
           gpointer      user_data)
{
	GList *children;
	GtkWidget *image;
	GIcon *icon, *icon2;
	const char *filename;

        /* Not a stock icon? */
	filename = g_object_get_data (G_OBJECT (item), "filename");
	if (filename == NULL)
		return;

	children = gtk_container_get_children (GTK_CONTAINER (item));
	image = children->data;
	g_assert (image != NULL);
	g_list_free (children);

	gtk_image_get_gicon (GTK_IMAGE (image), &icon, NULL);

	if (G_IS_EMBLEMED_ICON (icon))
		icon2 = g_emblemed_icon_get_icon (G_EMBLEMED_ICON (icon));
	else
		return;

	gtk_image_set_from_gicon (GTK_IMAGE (image), icon2, GTK_ICON_SIZE_DIALOG);
	g_object_unref (icon);
}

static void
set_tip (GtkWidget  *item,
         const char *tip,
         GEmblem    *emblem)
{
	GList *children;
	GtkWidget *image;
	GIcon *icon, *icon2;

	children = gtk_container_get_children (GTK_CONTAINER (item));
	image = children->data;
	g_assert (image != NULL);
	g_list_free (children);

	gtk_image_get_gicon (GTK_IMAGE (image), &icon, NULL);
	if (G_IS_EMBLEMED_ICON (icon)) {
		return;
	}

	icon2 = g_emblemed_icon_new (icon, emblem);
	gtk_image_set_from_gicon (GTK_IMAGE (image), icon2, GTK_ICON_SIZE_DIALOG);

	gtk_widget_set_tooltip_text (GTK_WIDGET (item), tip);
}

void
um_photo_dialog_set_user (UmPhotoDialog *um,
                          ActUser       *user)
{
	ActUserManager *manager;
	GSList *list, *l;
	ActUser *u;
	GIcon *icon;
	GEmblem *emblem;
	GList *children, *c;

	g_return_if_fail (um != NULL);

	if (um->user) {
		g_object_unref (um->user);
		um->user = NULL;
	}
	um->user = user;

	if (um->user) {
		g_object_ref (um->user);

		children = gtk_container_get_children (GTK_CONTAINER (um->photo_popup));
		g_list_foreach (children, (GFunc) clear_tip, NULL);

		manager = act_user_manager_get_default ();
		list = act_user_manager_list_users (manager);

		icon = g_themed_icon_new ("avatar-default");
		emblem = g_emblem_new (icon);
		g_object_unref (icon);

		for (l = list; l; l = l->next) {
			const char *filename;

			u = l->data;
			if (u == user)
				continue;
			filename = act_user_get_icon_file (u);
			if (filename  == NULL)
				continue;
			for (c = children; c; c = c->next) {
				const char *f;

				f = g_object_get_data (G_OBJECT (c->data), "filename");
				if (f == NULL)
					continue;
				if (strcmp (f, filename) == 0) {
					char *tip;

					tip = g_strdup_printf (_("Used by %s"),
						act_user_get_real_name (u));
					set_tip (GTK_WIDGET (c->data), tip, emblem);
					g_free (tip);
					break;
				}
			}
		}
		g_slist_free (list);

		g_object_unref (emblem);
	}
}
