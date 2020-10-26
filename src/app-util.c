#include "app-util.h"
#include "app-menu.h"

#define MENU_GLIB_STR_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

static char *app_xdg_icon_remove_extension (const char *icon)
{
    char *icon_no_extension;
    char *p;

    icon_no_extension = g_strdup (icon);
    p = strrchr (icon_no_extension, '.');
    if (p &&
        (strcmp (p, ".png") == 0 ||
         strcmp (p, ".xpm") == 0 ||
         strcmp (p, ".svg") == 0)) {
        *p = 0;
    }

    return icon_no_extension;
}

GIcon *app_gicon_from_icon_name (const char *icon_name) 
{
    GIcon *icon = NULL;
    if (icon_name == NULL) 
    {
        return NULL;
    }

    if (g_path_is_absolute(icon_name)) 
    {
        if (!g_file_test (icon_name, G_FILE_TEST_EXISTS)) 
        {
            gchar *name = g_path_get_basename (icon_name);
            icon = g_themed_icon_new (name);
            g_free (name);
        }
        else 
        {
            GFile *gfile = g_file_new_for_path (icon_name);
            icon = g_file_icon_new (gfile);
            g_object_unref (gfile);
        }
    }
    else 
    {
        gchar *name = app_xdg_icon_remove_extension (icon_name);
        icon = g_themed_icon_new (name);
        g_free (name);
    }
    return icon;
}
static gboolean
menu_util_query_tooltip_cb (GtkWidget  *widget,
                            gint        x,
                            gint        y,
                            gboolean    keyboard_tip,
                            GtkTooltip *tooltip,
                            const char *text)
{
    gtk_tooltip_set_text (tooltip, text);
    return TRUE;
}

void menu_util_set_tooltip_text (GtkWidget  *widget,
                                 const char *text)
{
    g_signal_handlers_disconnect_matched (widget,
                                          G_SIGNAL_MATCH_FUNC,
                                          0, 0, NULL,
                                          menu_util_query_tooltip_cb,
                                          NULL);

    if (g_utf8_strlen (text,-1) <= 14)
        return;
    if (MENU_GLIB_STR_EMPTY (text)) {
        g_object_set (widget, "has-tooltip", FALSE, NULL);
        return;
    }

    g_object_set (widget, "has-tooltip", TRUE, NULL);
    g_signal_connect_data (widget,
                          "query-tooltip",
                           G_CALLBACK (menu_util_query_tooltip_cb),
                           g_strdup (text),
                          (GClosureNotify) G_CALLBACK (g_free),
                           0);
}
GtkWidget *set_button_style(const gchar *icon_name)
{
    GtkWidget *button;
    GtkWidget *image;

    button = gtk_toggle_button_new ();
    image  = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (button), image);
    return button;
}
GtkWidget *
menu_error_dialog (GtkWindow  *parent,
                   GdkScreen  *screen,
                   const char *dialog_class,
                   gboolean    auto_destroy,
                   const char *primary_text,
                   const char *secondary_text)
{
    GtkWidget *dialog;
    char      *freeme;

    freeme = NULL;

    if (primary_text == NULL) {
        g_warning ("NULL dialog");
         /* No need to translate this, this should NEVER happen */
        freeme = g_strdup_printf ("Error with displaying error "
                      "for dialog of class %s",
                      dialog_class);
        primary_text = freeme;
    }

    dialog = gtk_message_dialog_new (parent, 0, GTK_MESSAGE_ERROR,
                     GTK_BUTTONS_CLOSE, "%s", primary_text);
    if (secondary_text != NULL)
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                              "%s", secondary_text);

    if (screen)
        gtk_window_set_screen (GTK_WINDOW (dialog), screen);

    if (!parent) {
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), FALSE);
        gtk_window_set_title (GTK_WINDOW (dialog), "Error");
    }

    gtk_widget_show_all (dialog);

    if (auto_destroy)
        g_signal_connect_swapped (G_OBJECT (dialog), "response",
                      G_CALLBACK (gtk_widget_destroy),
                      G_OBJECT (dialog));

    if (freeme)
        g_free (freeme);

    return dialog;
}
static void
_app_launch_error_dialog (const gchar *name,
                GdkScreen   *screen,
                const gchar *message)
{
    char *primary;

    if (name)
        primary = g_markup_printf_escaped ("Could not launch '%s'",
                           name);
    else
        primary = g_strdup ("Could not launch application");

    menu_error_dialog (NULL, screen, "cannot_launch", TRUE,
                primary, message);
    g_free (primary);
}

static gboolean
_app_launch_handle_error (const gchar  *name,
                          GdkScreen    *screen,
                          GError       *local_error,
                          GError      **error)
{
    if (g_error_matches (local_error,
                 G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
        g_error_free (local_error);
        return TRUE;
    }

    else if (error != NULL)
        g_propagate_error (error, local_error);

    else {
        _app_launch_error_dialog (name, screen, local_error->message);
        g_error_free (local_error);
    }

    return FALSE;
}
static void
dummy_child_watch (GPid     pid,
           gint     status,
           gpointer user_data)
{
    /* Nothing, this is just to ensure we don't double fork
     * and break pkexec:
     * https://bugzilla.gnome.org/show_bug.cgi?id=675789
     */
}

static void
gather_pid_callback (GDesktopAppInfo   *gapp,
             GPid               pid,
             gpointer           data)
{
    g_child_watch_add (pid, dummy_child_watch, NULL);
}
gboolean
menu_app_info_launch_uris (GDesktopAppInfo   *appinfo,
                           GList             *uris,
                           GdkScreen         *screen,
                           const gchar       *action,
                           guint32            timestamp,
                           GError            **error)
{
    GdkAppLaunchContext *context;
    GError              *local_error;
    gboolean             retval;
    GdkDisplay          *display;

    g_return_val_if_fail (G_IS_DESKTOP_APP_INFO (appinfo), FALSE);
    g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    display = gdk_display_get_default ();
    context = gdk_display_get_app_launch_context (display);
    gdk_app_launch_context_set_screen (context, screen);
    gdk_app_launch_context_set_timestamp (context, timestamp);

    local_error = NULL;
    if (action == NULL)
    {
        retval = g_desktop_app_info_launch_uris_as_manager (appinfo, uris,
                           G_APP_LAUNCH_CONTEXT (context),
                           G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                           NULL, NULL, gather_pid_callback, appinfo,
                           &local_error);
    }
    else
    {
        g_desktop_app_info_launch_action (appinfo, action, G_APP_LAUNCH_CONTEXT (context));
        retval = TRUE;
    }

    g_object_unref (context);

    if ((local_error == NULL) && (retval == TRUE))
        return TRUE;

    return _app_launch_handle_error (g_app_info_get_name (G_APP_INFO(appinfo)),
                       screen, local_error, error);
}
static void
menu_show_error_dialog (const gchar *uri,
                        GdkScreen   *screen,
                        const gchar *message)
{
    char *primary;

    primary = g_markup_printf_escaped (_("Could not open location '%s'"),uri);
    menu_error_dialog (NULL, screen, "cannot_show_url", TRUE,
                primary, message);
    g_free (primary);
}

gboolean
menu_show_uri_force_mime_type (GdkScreen    *screen,
                               const gchar  *uri,
                               const gchar  *mime_type,
                               guint32       timestamp,
                               GError      **error)
{
    GFile    *file;
    GAppInfo *app;
    gboolean  ret;
    GList    *uris = NULL;

    g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
    g_return_val_if_fail (uri != NULL, FALSE);
    g_return_val_if_fail (mime_type != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    file = g_file_new_for_uri (uri);
    app = g_app_info_get_default_for_type (mime_type,
                           !g_file_is_native (file));
    g_object_unref (file);

    if (app == NULL)
    {
        menu_show_error_dialog (uri,screen,_("No application to handle search folders is installed."));
        return FALSE;
    }
    uris = g_list_prepend (uris, (gpointer) uri);
    ret = menu_app_info_launch_uris ((GDesktopAppInfo*)app, uris,
                                     screen, NULL, timestamp, error);

    g_list_free (uris);
    g_object_unref (app);

    return ret;
}
typedef char * (*LookupInDir) (const char *basename, const char *dir);
static char *
lookup_in_applications_subdir (const char *basename,
                               const char *dir)
{
    char *path;

    path = g_build_filename (dir, "applications", basename, NULL);
    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_free (path);
        return NULL;
    }

    return path;
}

static char *
menu_lookup_in_data_dirs_internal (const char *basename,
                                   LookupInDir lookup)
{
    const char * const *system_data_dirs;
    const char          *user_data_dir;
    char                *retval;
    int                  i;

    user_data_dir    = g_get_user_data_dir ();
    system_data_dirs = g_get_system_data_dirs ();

    if ((retval = lookup (basename, user_data_dir)))
        return retval;

    for (i = 0; system_data_dirs[i]; i++)
        if ((retval = lookup (basename, system_data_dirs[i])))
            return retval;

    return NULL;
}


static char *
menu_lookup_in_applications_dirs (const char *basename)
{
    return menu_lookup_in_data_dirs_internal (basename,lookup_in_applications_subdir);
}


gboolean
menu_show_fm_search_uri (const gchar  *fm,
                         GdkScreen    *screen,
                         const gchar  *uri,
                         guint32       timestamp,
                         GError      **error)
{
    char            *desktopfile;
    GDesktopAppInfo *appinfo = NULL;
    gboolean         ret;
    GList           *uris = NULL;

    desktopfile = menu_lookup_in_applications_dirs (fm);
    if (desktopfile)
    {
        appinfo = g_desktop_app_info_new_from_filename (desktopfile);
        g_free (desktopfile);
    }

    if (!appinfo)
    {
        menu_show_error_dialog (uri, screen,
                                _("No application to handle search folders is installed."));
        return FALSE;
    }
    uris = g_list_prepend (uris, (gpointer) uri);
    ret = menu_app_info_launch_uris (appinfo, uris,
                                     screen, NULL, timestamp, error);
    g_object_unref (appinfo);
    g_list_free (uris);
    return ret;
}
