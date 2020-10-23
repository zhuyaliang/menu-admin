#ifndef __APP_UTIL__
#define __APP_UTIL__

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#define  MENU_ADMID_SCHEMA                 "org.admin.menu"
#define  MENU_DEFAULT_ITEM                 "default-item"
#define  MENU_ICON_SIZE                    "icon-size"
#define  MENU_FONT_SIZE                    "font-size"


typedef GDesktopAppInfo GDAInfo;
GIcon     *app_gicon_from_icon_name         (const gchar *icon_name);

void       menu_util_set_tooltip_text       (GtkWidget   *widget,
                                             const gchar *text);

GtkWidget *set_button_style                 (const gchar *icon_name);

gboolean   menu_app_info_launch_uris        (GDAInfo     *appinfo,
                                             GList       *uris,
                                             GdkScreen   *screen,
                                             const gchar *action,
                                             guint32      timestamp,
                                             GError      **error);

gboolean   menu_show_uri_force_mime_type    (GdkScreen    *screen,
                                             const gchar  *uri,
                                             const gchar  *mime_type,
                                             guint32       timestamp,
                                             GError      **error);

GtkWidget *menu_error_dialog                (GtkWindow    *parent,
                                             GdkScreen    *screen,
                                             const char   *dialog_class,
                                             gboolean      auto_destroy,
                                             const char   *primary_text,
                                             const char   *secondary_text);
#endif
