#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gdm/gdm-user-switching.h>
#include <act/act-user-manager.h>
#include <act/act-user.h>

#include "menu-tool.h"
#include "app-util.h"
#include "menu-lockdown.h"

#define  DMFILE        "/etc/systemd/system/display-manager.service"
#define  MATE_DESKTOP  "/usr/share/applications/matecc.desktop"
#define  GNOME_DESKTOP "/usr/share/applications/gnome-control-center.desktop"

struct _MenuSessionManagerPrivate {
    GDBusProxy *session_proxy;
    GDBusProxy *screen;
};

G_DEFINE_TYPE (MenuSessionManager, menu_session_manager, G_TYPE_OBJECT);
typedef enum
{
    GDM,
    LIGHTDM,
    ERROR,
}dm_type;

static dm_type get_dm_type (void)
{
    GKeyFile  *key_file;
    gboolean   loaded;
    gchar     *exec;

    key_file = g_key_file_new ();
    loaded = g_key_file_load_from_file (key_file,
                                        DMFILE,
                                        G_KEY_FILE_NONE,
                                        NULL);
    if (!loaded)
    {
        g_key_file_free (key_file);
        return ERROR;
    }
    exec = g_key_file_get_string (key_file, "Service", "ExecStart", NULL);

    if (strstr (exec, "gdm") != NULL)
        return GDM;
    else if (strstr (exec, "lightdm") != NULL)
        return LIGHTDM;
    else
        return ERROR;
}
static gboolean get_desktop_type (void)
{
    const char *desktop = g_getenv ("XDG_CURRENT_DESKTOP");

    if (desktop == NULL)
        return FALSE;

    if (g_strcmp0 (desktop,"MATE") == 0)
        return TRUE;
    else
        return FALSE;

}
static void users_loaded (ActUserManager  *manager,
                          GParamSpec      *pspec,
                          gpointer         user_data)
{
    GtkWidget  *content_area;
    GtkWidget  *box;
    ActUser    *user = NULL;
    GtkWidget  *dialog = GTK_WIDGET (user_data);
    const char *user_name;
    const char *icon_name;
    gint64      login_time;
    const char *home_path;
    const char *session_name;
    GSList     *list = NULL,*l;
    GdkPixbuf  *pb2;
    g_autoptr(GdkPixbuf) pb1 = NULL;
    GtkWidget  *image;
    GtkWidget  *table;
    GtkWidget  *label;
    GtkWidget  *entry;
    GDateTime  *time;
    g_autofree gchar *mod_date = NULL;

    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width (GTK_CONTAINER (box), 8);
    gtk_box_pack_start (GTK_BOX (content_area), box, FALSE, FALSE, 0);

    user_name = g_get_user_name ();
    list = act_user_manager_list_users (manager);
    for (l = list; l; l = l->next)
    {
        user = l->data;
        user_name = act_user_get_user_name (user);
        if (g_strcmp0 (g_get_user_name (),act_user_get_user_name (user)) == 0)
            break;
    }
    if (user == NULL)
        return;
    icon_name = act_user_get_icon_file (user);
    pb1 = gdk_pixbuf_new_from_file(icon_name, NULL);
    pb2 = gdk_pixbuf_scale_simple (pb1, 96, 96, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(pb2);

    gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table), 8);
    gtk_grid_set_column_spacing (GTK_GRID (table), 8);
    gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 12);

    label = gtk_label_new_with_mnemonic (_("User name"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), user_name);
    gtk_widget_set_sensitive (entry, FALSE);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 0, 1, 1);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    label = gtk_label_new_with_mnemonic (_("Home path"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    entry = gtk_entry_new ();
    gtk_widget_set_sensitive (entry, FALSE);
    home_path = act_user_get_home_dir (user);
    gtk_entry_set_text (GTK_ENTRY (entry), home_path);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 1, 1, 1);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    label = gtk_label_new_with_mnemonic (_("Login time"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);
    entry = gtk_entry_new ();
    gtk_widget_set_sensitive (entry, FALSE);
    login_time = act_user_get_login_time (user);
    time = g_date_time_new_from_unix_local (login_time);
    mod_date = g_date_time_format (time, "%Y %b %d %H:%M:%S");
    gtk_entry_set_text (GTK_ENTRY (entry), mod_date);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 2, 1, 1);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    label = gtk_label_new_with_mnemonic (_("Session name"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 3, 1, 1);
    entry = gtk_entry_new ();
    gtk_widget_set_sensitive (entry, FALSE);
    session_name = act_user_get_x_session (user);
    gtk_entry_set_text (GTK_ENTRY (entry), session_name);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 3, 1, 1);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    gtk_widget_show_all (dialog);
}

static void
user_info_cb (GtkDialog *dialog,
              gint       response_id,
              gpointer   user_data)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}
void
system_user_info (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
    GtkWidget  *dialog;
    gboolean    loaded;
    GtkWindow  *parent = GTK_WINDOW (user_data);
    ActUserManager *manager;

    dialog = gtk_dialog_new_with_buttons (_("User Info"),
                                          parent,
                                          GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
                                          _("_OK"),
                                          GTK_RESPONSE_OK,
                                          _("_Cancel"),
                                          GTK_RESPONSE_CANCEL,
                                          NULL);

    g_signal_connect (dialog,
                     "response",
                      G_CALLBACK (user_info_cb),
                      NULL);
    gtk_container_set_border_width(GTK_CONTAINER (dialog), 20);
    gtk_window_set_deletable(GTK_WINDOW (dialog), FALSE);
    gtk_window_set_default_size (GTK_WINDOW (dialog), 450, 200);

    manager = act_user_manager_get_default ();
    g_object_get (manager, "is-loaded", &loaded, NULL);
    if (loaded)
        users_loaded (manager, NULL, dialog);
    else
        g_signal_connect(manager,
                        "notify::is-loaded",
                         G_CALLBACK (users_loaded),
                         dialog);

}
void
system_settings (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
    GtkWindow  *window = GTK_WINDOW (user_data);
    GdkScreen  *screen;
    gboolean    type;

    screen = gtk_window_get_screen (window);
    type = get_desktop_type ();
    if (type)
        app_launch_desktop_file (MATE_DESKTOP, screen, NULL);
    else
        app_launch_desktop_file (GNOME_DESKTOP, screen, NULL);
}
void
system_switch_user (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
    dm_type type;
    const char *xdg_seat_path = g_getenv ("XDG_SEAT_PATH");
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START;
    GDBusProxy *proxy = NULL;
    GError *error = NULL;

    type = get_dm_type ();

    if (type == ERROR)
    {
        return;
    }
    if (type == GDM)
    {
        gdm_goto_login_session_sync (NULL, &error);
    }
    if (type == LIGHTDM)
    {

        proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                              flags,
                                              NULL,
                                              "org.freedesktop.DisplayManager",
                                              xdg_seat_path,
                                              "org.freedesktop.DisplayManager.Seat",
                                              NULL,
                                              &error);
        if (proxy != NULL)
        {
            g_dbus_proxy_call_sync (proxy,
                                    "SwitchToGreeter",
                                    g_variant_new ("()"),
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    NULL);
            g_object_unref (proxy);
        }
    }
    if (error != NULL)
    {
        g_debug ("Error switching user: %s", error->message);
        g_error_free (error);
    }
}
static void
logout_ready_callback (GObject      *source_object,
                       GAsyncResult *res,
                       gpointer      user_data)
{
    MenuSessionManager *manager = (MenuSessionManager *) user_data;
    GError *error = NULL;
    GVariant *ret;

    ret = g_dbus_proxy_call_finish (manager->priv->session_proxy, res, &error);
    if (ret)
    {
        g_variant_unref (ret);
    }

    if (error)
    {
        g_warning ("Could not ask session manager to log out: %s", error->message);
        g_error_free (error);
    }
}
void
system_log_out (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
    MenuSessionManager *manager = NULL;
    MenuSessionManagerLogoutType  mode = MENU_SESSION_MANAGER_LOGOUT_MODE_NORMAL;
    manager = menu_session_manager_get ();

    if (!manager->priv->session_proxy)
    {
         g_warning ("Session manager service not available.");
         return;
    }

    g_dbus_proxy_call (manager->priv->session_proxy,
                       "Logout",
                       g_variant_new ("(u)", mode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       (GAsyncReadyCallback) logout_ready_callback,
                       manager);
}

static void
lock_ready_callback (GObject      *source_object,
                     GAsyncResult *res,
                     gpointer      user_data)
{
    GDBusProxy *proxy = G_DBUS_PROXY (user_data);
    GError     *error = NULL;
    GVariant   *ret;

    ret = g_dbus_proxy_call_finish (proxy, res, &error);
    if (ret)
    {
        g_variant_unref (ret);
    }

    if (error)
    {
        g_warning ("Could not ask screensaver to lock: %s",
            error->message);
        g_error_free (error);
    }
}
void
system_lock_screen (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
    MenuSessionManager *manager = NULL;
    GDBusProxy *proxy;

    manager = menu_session_manager_get ();
    proxy = manager->priv->screen;
    g_dbus_proxy_call (proxy,
                      "Lock",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       (GAsyncReadyCallback) lock_ready_callback,
                       proxy);

}
static void
reboot_ready_callback (GObject      *source_object,
                       GAsyncResult *res,
                       gpointer      user_data)
{
    GDBusProxy *proxy = G_DBUS_PROXY (user_data);
    GError     *error = NULL;
    GVariant   *ret;

    ret = g_dbus_proxy_call_finish (proxy, res, &error);
    if (ret)
    {
        g_variant_unref (ret);
    }

    if (error)
    {
        g_warning ("Could not ask session manager to reboot: %s", error->message);
        g_error_free (error);
    }
}
void
system_reboot (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
    MenuSessionManager *manager = NULL;
    gboolean mate_desktop;

    manager = menu_session_manager_get ();
    mate_desktop = get_desktop_type ();

    if (!manager->priv->session_proxy)
    {
        g_warning ("Session manager service not available.");
        return;
    }
    if (mate_desktop)
    {
        g_dbus_proxy_call (manager->priv->session_proxy,
                           "Shutdown",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) reboot_ready_callback,
                           manager->priv->session_proxy);

    }
    else
    {
        g_dbus_proxy_call (manager->priv->session_proxy,
                           "Reboot",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) reboot_ready_callback,
                           manager->priv->session_proxy);

    }
}
static void
shutdown_ready_callback (GObject      *source_object,
                         GAsyncResult *res,
                         gpointer      user_data)
{
    GDBusProxy *proxy = G_DBUS_PROXY (user_data);
    GError     *error = NULL;
    GVariant   *ret;

    ret = g_dbus_proxy_call_finish (proxy, res, &error);
    if (ret)
    {
        g_variant_unref (ret);
    }

    if (error)
    {
        g_warning ("Could not ask session manager to shutdown: %s", error->message);
        g_error_free (error);
    }
}
void
system_shutdown (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
    MenuSessionManager *manager = NULL;
    manager = menu_session_manager_get ();

    if (!manager->priv->session_proxy)
    {
        g_warning ("Session manager service not available.");
        return;
    }

    g_dbus_proxy_call (manager->priv->session_proxy,
                       "Shutdown",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       (GAsyncReadyCallback) shutdown_ready_callback,
                       manager->priv->session_proxy);
}

static gboolean get_lock_screen_visible (void)
{
    GDBusProxy *proxy_gnome;
    GDBusProxy *proxy_mate;
    gboolean g = FALSE;
    gboolean m = FALSE;

    proxy_gnome = g_dbus_proxy_new_for_bus_sync (
                        G_BUS_TYPE_SESSION,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.gnome.ScreenSaver",
                        "/org/gnome/ScreenSaver",
                        "org.gnome.ScreenSaver",
                        NULL, NULL);

    proxy_mate = g_dbus_proxy_new_for_bus_sync (
                        G_BUS_TYPE_SESSION,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.gnome.ScreenSaver",
                        "/org/gnome/ScreenSaver",
                        "org.gnome.ScreenSaver",
                        NULL, NULL);

    if (proxy_mate != NULL)
    {
        g_object_unref (proxy_mate);
        m = TRUE;
    }
    if (proxy_gnome != NULL)
    {
        g_object_unref (proxy_gnome);
        g = TRUE;
    }

    return m | g;
}
void set_system_lockdown (GtkBuilder *builder)
{
    GtkWidget    *widget;
    MenuLockdown *lockdown;
    gboolean      enable;

    lockdown = menu_lockdown_get ();

    widget = (GtkWidget *)gtk_builder_get_object (builder, "switch-user");

    g_object_bind_property (lockdown,
                           "disable-switch-user",
                            widget,
                           "visible",
                            G_BINDING_SYNC_CREATE|G_BINDING_INVERT_BOOLEAN);

    widget = (GtkWidget *)gtk_builder_get_object (builder, "log-out");

    g_object_bind_property (lockdown,
                           "disable-log-out",
                            widget,
                           "visible",
                            G_BINDING_SYNC_CREATE|G_BINDING_INVERT_BOOLEAN);

    widget = (GtkWidget *)gtk_builder_get_object (builder, "lock-screen");

    g_object_bind_property (lockdown,
                           "disable-lock-screen",
                            widget,
                           "visible",
                            G_BINDING_SYNC_CREATE|G_BINDING_INVERT_BOOLEAN);

    enable = get_lock_screen_visible ();
    gtk_widget_set_visible (widget, enable);
}

static void
menu_session_manager_dispose (GObject *object)
{
    MenuSessionManager *manager;

    manager = MENU_SESSION_MANAGER (object);

    g_object_unref (manager->priv->session_proxy);
    g_object_unref (manager->priv->screen);

    G_OBJECT_CLASS (menu_session_manager_parent_class)->dispose (object);
}
static void
menu_session_manager_class_init (MenuSessionManagerClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->dispose = menu_session_manager_dispose;
}

static void
menu_session_manager_init (MenuSessionManager *manager)
{
    GError  *error = NULL;
    gboolean desktop_type;

    manager->priv = menu_session_manager_get_instance_private (manager);

    manager->priv->session_proxy = g_dbus_proxy_new_for_bus_sync (
                        G_BUS_TYPE_SESSION,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.gnome.SessionManager",
                        "/org/gnome/SessionManager",
                        "org.gnome.SessionManager",
                        NULL, &error);

    if (error)
    {
        g_warning ("Could not connect to session manager: %s",
               error->message);
        g_error_free (error);
    }

    desktop_type = get_desktop_type ();
    if (desktop_type)
    {
        manager->priv->screen = g_dbus_proxy_new_for_bus_sync (
                            G_BUS_TYPE_SESSION,
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.mate.ScreenSaver",
                            "/",
                            "org.mate.ScreenSaver",
                            NULL, NULL);

    }
    else
    {
        manager->priv->screen = g_dbus_proxy_new_for_bus_sync (
                        G_BUS_TYPE_SESSION,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.gnome.ScreenSaver",
                        "/org/gnome/ScreenSaver",
                        "org.gnome.ScreenSaver",
                        NULL, NULL);
    }
}

MenuSessionManager *
menu_session_manager_get (void)
{
    static MenuSessionManager *manager = NULL;

    if (manager == NULL)
    {
        manager = g_object_new (MENU_TYPE_SESSION_MANAGER, NULL);
    }
    return manager;
}
