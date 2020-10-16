#ifndef __APP_UTIL__
#define __APP_UTIL__

#include <gtk/gtk.h>
#include <gio/gio.h>

GIcon     *app_gicon_from_icon_name    (const gchar *icon_name);

void       menu_util_set_tooltip_text  (GtkWidget   *widget,
                                        const gchar *text);

GtkWidget *set_button_style            (const gchar *button_text,
                                        const gchar *icon_name);
#endif
