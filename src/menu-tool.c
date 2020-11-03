#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "menu-tool.h"
#include "menu-lockdown.h"

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

}
