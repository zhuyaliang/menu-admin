#ifndef __MENU_TOOL__
#define __MENU_TOOL__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MENU_TYPE_SESSION_MANAGER      (menu_session_manager_get_type ())
#define MENU_SESSION_MANAGER(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), MENU_TYPE_SESSION_MANAGER, MenuSessionManager))

typedef struct _MenuSessionManager         MenuSessionManager;
typedef struct _MenuSessionManagerClass    MenuSessionManagerClass;
typedef struct _MenuSessionManagerPrivate  MenuSessionManagerPrivate;

typedef enum {
        MENU_SESSION_MANAGER_LOGOUT_MODE_NORMAL = 0,
        MENU_SESSION_MANAGER_LOGOUT_MODE_NO_CONFIRMATION,
        MENU_SESSION_MANAGER_LOGOUT_MODE_FORCE
} MenuSessionManagerLogoutType;

struct _MenuSessionManager {
    GObject parent;

    /*< private > */
    MenuSessionManagerPrivate *priv;
};

struct _MenuSessionManagerClass {
    GObjectClass parent_class;
};

GType menu_session_manager_get_type (void);

MenuSessionManager *menu_session_manager_get (void);


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

G_END_DECLS
#endif
