#ifndef __APP_UTIL__
#define __APP_UTIL__

#include <gtk/gtk.h>
#include <gio/gio.h>

GIcon    *app_gicon_from_icon_name    (const char *icon_name);

void      menu_util_set_tooltip_text  (GtkWidget  *widget,
                                       const char *text);

#endif
