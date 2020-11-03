#ifndef __MENU_LOCKDOWN_H__
#define __MENU_LOCKDOWN_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define MENU_TYPE_LOCKDOWN            (menu_lockdown_get_type ())
#define MENU_LOCKDOWN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MENU_TYPE_LOCKDOWN, MenuLockdown))
#define MENU_IS_LOCKDOWN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MENU_TYPE_LOCKDOWN))

typedef struct _MenuLockdown          MenuLockdown;
typedef struct _MenuLockdownClass     MenuLockdownClass;
typedef struct _MenuLockdownPrivate   MenuLockdownPrivate;

struct _MenuLockdown
{
    GObject parent;
    
    /*< private > */
    MenuLockdownPrivate *priv;
};

struct _MenuLockdownClass
{
    GObjectClass parent_class;
};

GType           menu_lockdown_get_type   (void);

MenuLockdown   *menu_lockdown_get        (void);

G_END_DECLS

#endif /* __MENU_LOCKDOWN_H__ */
