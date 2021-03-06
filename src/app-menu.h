#ifndef __APP_MENU__
#define __APP_MENU__

#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>


#pragma once
#define APP_TYPE_MENU (app_menu_get_type ())        
G_DECLARE_FINAL_TYPE (AppMenu, app_menu, APP, MENU, GObject)

AppMenu        *app_menu_new                 (void);

AppMenu        *create_applications_menu     (const char    *menu_file,
                                              GtkContainer  *container,
                                              GtkBox        *box);

GtkWidget       *get_menu_app_search_tree    (AppMenu       *menu);

GtkListStore    *get_menu_app_search_store   (AppMenu       *menu);

int              view_search_app_results     (AppMenu       *menu,
                                              const char    *text);
#endif
