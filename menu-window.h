#ifndef __MENU_WINDOW__
#define __MENU_WINDOW__

#include <gtk/gtk.h>

#define MENU_TYPE_WINDOW         (menu_window_get_type ())
#define MENU_WINDOW(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), MENU_TYPE_WINDOW, MenuWindow))
#define MENU_WINDOW_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), MENU_TYPE_WINDOW, MenuWindowClass))
#define MENU_IS_WINDOW(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), MENU_TYPE_WINDOW))
#define MENU_IS_WINDOW_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), MENU_TYPE_WINDOW))
#define MENU_WINDOW_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), MENU_TYPE_WINDOW, MenuWindowClass))

typedef struct _MenuWindow        MenuWindow;
typedef struct _MenuWindowClass   MenuWindowClass;
typedef struct _MenuWindowPrivate MenuWindowPrivate;

struct _MenuWindow {
    GtkWindow               parent_instance;
    MenuWindowPrivate  *priv;
};

struct _MenuWindowClass {
    GtkWindowClass parent_class;
};
GType         menu_window_get_type         (void) G_GNUC_CONST;

GtkWidget    *menu_window_new              (void);

#endif
