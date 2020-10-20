#ifndef __APP_MENU__
#define __APP_MENU__

#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h>
#pragma once
#define APP_TYPE_MENU (app_menu_get_type ())        
G_DECLARE_FINAL_TYPE (AppMenu, app_menu, APP, MENU, GObject)

AppMenu        *app_menu_new                 (void);

AppMenu        *create_applications_menu     (const char    *menu_file,
                                              GtkContainer *container,
                                              GtkBox       *box);

GtkWidget *get_menu_app_tree (AppMenu *menu);
#endif
