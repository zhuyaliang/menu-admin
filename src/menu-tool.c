#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gdm/gdm-user-switching.h>
#include <lightdm.h>
#include "menu-tool.h"
#include "menu-lockdown.h"

#define  DMFILE    "/etc/systemd/system/display-manager.service"

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
void
system_user_info (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
}
void
system_settings (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
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
void
system_log_out (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
}
void
system_lock_screen (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
}
void
system_suspend (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
}
void
system_reboot (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
}
void
system_shutdown (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
}

void set_system_lockdown (GtkBuilder *builder)
{
    GtkWidget    *widget;
    MenuLockdown *lockdown;

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

}
