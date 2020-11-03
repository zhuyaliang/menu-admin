#include "menu-lockdown.h"

#define MENU_DESKTOP_LOCKDOWN_SCHEMA          "org.gnome.desktop.lockdown"
#define MENU_DESKTOP_DISABLE_LOCK_SCREEN_KEY  "disable-lock-screen"
#define MENU_DESKTOP_DISABLE_LOG_OUT_KEY      "disable-log-out"
#define MENU_DESKTOP_DISABLE_SWITCH_USER_KEY  "disable-user-switching"

struct _MenuLockdownPrivate
{
    GSettings *desktop_settings;
    gboolean   disable_lock_screen;
    gboolean   disable_log_out;
    gboolean   disable_switch_user;
};

enum
{
    PROP_0,
    PROP_DISABLE_LOCK_SCREEN,
    PROP_DISABLE_LOG_OUT,
    PROP_DISABLE_SWITCH_USER,
};

G_DEFINE_TYPE_WITH_PRIVATE (MenuLockdown, menu_lockdown, G_TYPE_OBJECT)

static GObject *
menu_lockdown_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_properties)
{
    GObject       *obj;
    MenuLockdown *lockdown;

    obj = G_OBJECT_CLASS (menu_lockdown_parent_class)->constructor (type,
                                                                    n_construct_properties,
                                                                    construct_properties);

    lockdown = MENU_LOCKDOWN (obj);

    lockdown->priv->desktop_settings = g_settings_new (MENU_DESKTOP_LOCKDOWN_SCHEMA);
    g_settings_bind (lockdown->priv->desktop_settings,
                     MENU_DESKTOP_DISABLE_LOCK_SCREEN_KEY,
                     lockdown,
                     "disable-lock-screen",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (lockdown->priv->desktop_settings,
                     MENU_DESKTOP_DISABLE_LOG_OUT_KEY,
                     lockdown,
                     "disable-log-out",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (lockdown->priv->desktop_settings,
                     MENU_DESKTOP_DISABLE_SWITCH_USER_KEY,
                     lockdown,
                     "disable-switch-user",
                     G_SETTINGS_BIND_GET);

    return obj;
}

static void
menu_lockdown_set_property_helper (MenuLockdown *lockdown,
                                     gboolean      *field,
                                     const GValue  *value,
                                     const char    *property)
{
    gboolean new;

    new = g_value_get_boolean (value);
    if (new == *field)
        return;

    *field = new;
    g_object_notify (G_OBJECT (lockdown), property);
}

static void
menu_lockdown_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    MenuLockdown *lockdown;

    g_return_if_fail (MENU_IS_LOCKDOWN (object));

    lockdown = MENU_LOCKDOWN (object);

    switch (prop_id)
    {
        case PROP_DISABLE_LOCK_SCREEN:
            menu_lockdown_set_property_helper (lockdown,
                                               &lockdown->priv->disable_lock_screen,
                                               value,
                                               "disable-lock-screen");
            break;
        case PROP_DISABLE_LOG_OUT:
            menu_lockdown_set_property_helper (lockdown,
                                               &lockdown->priv->disable_log_out,
                                               value,
                                               "disable-log-out");
            break;
        case PROP_DISABLE_SWITCH_USER:
            menu_lockdown_set_property_helper (lockdown,
                                               &lockdown->priv->disable_switch_user,
                                               value,
                                              "disable-switch-user");
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
menu_lockdown_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    MenuLockdown *lockdown;

    g_return_if_fail (MENU_IS_LOCKDOWN (object));

    lockdown = MENU_LOCKDOWN (object);

    switch (prop_id)
    {
        case PROP_DISABLE_LOCK_SCREEN:
            g_value_set_boolean (value, lockdown->priv->disable_lock_screen);
            break;
        case PROP_DISABLE_LOG_OUT:
            g_value_set_boolean (value, lockdown->priv->disable_log_out);
            break;
        case PROP_DISABLE_SWITCH_USER:
            g_value_set_boolean (value, lockdown->priv->disable_switch_user);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
menu_lockdown_dispose (GObject *object)
{
    MenuLockdown *lockdown;

    lockdown = MENU_LOCKDOWN (object);

    if (lockdown->priv->desktop_settings)
        g_object_unref (lockdown->priv->desktop_settings);
    lockdown->priv->desktop_settings = NULL;

    G_OBJECT_CLASS (menu_lockdown_parent_class)->dispose (object);
}

static void
menu_lockdown_init (MenuLockdown *lockdown)
{
    lockdown->priv = menu_lockdown_get_instance_private (lockdown);
}

static void
menu_lockdown_class_init (MenuLockdownClass *lockdown_class)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (lockdown_class);

    gobject_class->constructor  = menu_lockdown_constructor;
    gobject_class->set_property = menu_lockdown_set_property;
    gobject_class->get_property = menu_lockdown_get_property;
    gobject_class->dispose      = menu_lockdown_dispose;

    g_object_class_install_property (
            gobject_class,
            PROP_DISABLE_LOCK_SCREEN,
            g_param_spec_boolean (
                    "disable-lock-screen",
                    "Disable lock screen",
                    "Whether lock screen is disabled or not",
                    TRUE,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
            gobject_class,
            PROP_DISABLE_LOG_OUT,
            g_param_spec_boolean (
                    "disable-log-out",
                    "Disable log out",
                    "Whether log out is disabled or not",
                    TRUE,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
            gobject_class,
            PROP_DISABLE_SWITCH_USER,
            g_param_spec_boolean (
                    "disable-switch-user",
                    "Disable user switching",
                    "Whether user switching is disabled or not",
                    TRUE,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

MenuLockdown *
menu_lockdown_get (void)
{
    MenuLockdown *shared_lockdown = NULL;
    
    shared_lockdown = g_object_new (MENU_TYPE_LOCKDOWN, NULL);

    return shared_lockdown;
}
