/*
*
*  Copyright 2017 Eero Talus
*
*  This file is part of Open Image Pipeline.
*
*  Open Image Pipeline is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Open Image Pipeline is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#define PRINT_IDENTIFIER "oipgui"

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <oipbuildinfo/oipbuildinfo.h>
#include <oipcore/abi/output.h>
#include <oipcore/oip.h>

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

#define OIPGUI_ADD_GTK_CALLBACK(callback)					\
	gtk_builder_add_callback_symbol(builder, QUOTE(callback), callback);	\

#define OIPGUI_BUILDER_DIR    "src/oipgui/gui"
#define OIPGUI_WINDOW_DEF_W   800
#define OIPGUI_WINDOW_DEF_H   500

#define WINDOW_MAIN  "window_main"
#define DIALOG_ABOUT "dialog_about"

static GObject *window = NULL;
static GObject *dialog_about = NULL;
static GtkBuilder *builder = NULL;

static void oipgui_about_dialog_show(void);
static void oipgui_about_dialog_setup(void);
static void oipgui_connect_signals(void);
static void oipgui_cleanup(void);

static void oipgui_about_dialog_show(void) {
	/*
	*  Show the About dialog.
	*/
	printverb("Show the about dialog.\n")
	gtk_dialog_run(GTK_DIALOG(dialog_about));
	printverb("Hide the about dialog.\n");
	gtk_widget_hide(GTK_WIDGET(dialog_about));
}

static void oipgui_about_dialog_setup(void) {
	/*
	*  Setup the About dialog.
	*/
	char *build_version = NULL;
	dialog_about = gtk_builder_get_object(builder, DIALOG_ABOUT);
	build_version = build_get_version_string("", &OIP_BUILD_INFO);
	if (!build_version) {
		printerr("Failed to construct version string.")
		return;
	}
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog_about),
					build_version);
	free(build_version);
}

static void oipgui_connect_signals(void) {
	/*
	*  Connect all GTK signals.
	*/
	OIPGUI_ADD_GTK_CALLBACK(oipgui_about_dialog_show);
	gtk_builder_connect_signals(builder, NULL);
}

static void oipgui_cleanup(void) {
	/*
	*  Free all allocated resources.
	*/
	printverb("Cleanup\n");
	if (dialog_about) {
		gtk_widget_destroy(GTK_WIDGET(dialog_about));
		dialog_about = NULL;
	}
	if (window) {
		gtk_widget_destroy(GTK_WIDGET(window));
		window = NULL;
	}
}

int main(int argc, char **argv) {
	GdkGeometry hints;

	if (oip_setup(argc, argv) != 0) {
		return 1;
	}

	gtk_init(&argc, &argv);

	builder = gtk_builder_new_from_file(OIPGUI_BUILDER_DIR"/oipgui.glade");
	window = gtk_builder_get_object(builder, WINDOW_MAIN);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	// Set window hints.
	hints.min_width = OIPGUI_WINDOW_DEF_W;
	hints.min_height = OIPGUI_WINDOW_DEF_H;
	gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL,
					&hints, GDK_HINT_MIN_SIZE);
	gtk_window_maximize(GTK_WINDOW(window));

	oipgui_about_dialog_setup();
	oipgui_connect_signals();

	// Enter the main loop.
	gtk_main();

	oipgui_cleanup();
	oip_cleanup();
	return 0;
}
