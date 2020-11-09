#ifndef __APP_UTIL__
#define __APP_UTIL__

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#define  MENU_ADMID_SCHEMA                 "org.admin.menu"

#define  MENU_DEFAULT_ITEM                 "default-item"
#define  MENU_ICON_SIZE                    "icon-size"
#define  MENU_FONT_SIZE                    "font-size"
#define  MENU_WIDTH_SIZE                   "width-size"
#define  MENU_COLUMN_SPACING               "column-spacing"
#define  MENU_ROW_SPACING                  "row-spacing"


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

gboolean   menu_show_fm_search_uri          (const gchar  *fm,
                                             GdkScreen    *screen,
                                             const gchar  *uri,
                                             guint32       timestamp,
                                             GError      **error);

gboolean   app_launch_desktop_file          (const char   *desktop_file,
                                             GdkScreen    *screen,
                                             GError      **error);

char      *menu_get_desktop_path_from_name  (const char   *dir,
                                             const char   *source);

gboolean   menu_desktop_file_copy           (const char   *source_path,
                                             const char   *target_path,
                                             GError      **error);

#endif
