#ifndef __MENU_TOOL__
#define __MENU_TOOL__

#include <gtk/gtk.h>


void        system_user_info           (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_settings            (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_switch_user         (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_log_out             (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_lock_screen         (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_suspend             (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_reboot              (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        system_shutdown            (GSimpleAction   *action,
                                        GVariant        *parameter,
                                        gpointer         user_data);

void        set_system_lockdown        (GtkBuilder      *builder);
#endif
