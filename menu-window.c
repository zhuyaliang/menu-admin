#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "menu-window.h"
#include "app-menu.h"


struct _MenuWindowPrivate 
{
    AppMenu    *menu;
    GSettings  *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE (MenuWindow, menu_window, GTK_TYPE_WINDOW)

static void set_box_background (GtkWidget *box)
{
    GtkCssProvider  *provider;
    GtkStyleContext *context;
    gchar           *css = NULL;

    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (box);
    css = g_strdup_printf ("window {background-color:rgba(252,252,252,252)}");
    gtk_css_provider_load_from_data (provider, css, -1, NULL);
    gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER (provider),
		                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);
    g_free (css);
}
static void
menu_window_fill (MenuWindow *menuwin)
{
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *category_box,*app_vbox;
    GtkWidget *toplevel;
    GdkScreen *screen;
    GdkVisual *visual;
    GtkWidget *scroll;

    set_box_background (GTK_WIDGET (menuwin));
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
    gtk_container_add (GTK_CONTAINER (menuwin), frame);
    
    vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    
    toplevel = gtk_widget_get_toplevel (frame);
    screen = gtk_widget_get_screen(GTK_WIDGET(toplevel));
    visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(toplevel), visual);
    
    category_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), category_box,FALSE,FALSE,0);
    
    app_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), app_vbox,FALSE,FALSE,0);
    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                    GTK_POLICY_NEVER, 
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (app_vbox), scroll, TRUE, TRUE, 0);
    gtk_widget_show_all (frame);
    menuwin->priv->menu = create_applications_menu ("mate-applications.menu",GTK_CONTAINER(scroll),GTK_BOX(category_box));
}

static GObject *
menu_window_constructor (GType                  type,
                         guint                  n_construct_properties,
                         GObjectConstructParam *construct_properties)
{
    GObject        *obj;
    MenuWindow *menuwin;

    obj = G_OBJECT_CLASS (menu_window_parent_class)->constructor (type,
                                      n_construct_properties,
                                      construct_properties);

    menuwin = MENU_WINDOW (obj);
    menu_window_fill (menuwin);

    return obj;
}
static void
menu_window_dispose (GObject *object)
{
    MenuWindow *menuwin;

    menuwin = MENU_WINDOW (object);
    if (menuwin->priv->settings)
        g_object_unref (menuwin->priv->settings);
    menuwin->priv->settings = NULL;
    G_OBJECT_CLASS (menu_window_parent_class)->dispose (object);
}

static void
menu_window_class_init (MenuWindowClass *klass)
{
    GObjectClass   *gobject_class   = G_OBJECT_CLASS (klass);

    gobject_class->constructor = menu_window_constructor;
    gobject_class->dispose = menu_window_dispose;
}

static void
menu_window_init (MenuWindow *menuwin)
{
    GtkWindow *window;

    menuwin->priv = menu_window_get_instance_private (menuwin);

    window = GTK_WINDOW (menuwin);
    gtk_window_set_type_hint (window, GDK_WINDOW_TYPE_HINT_MENU);
    gtk_window_set_position (window,GTK_WIN_POS_MOUSE);
    gtk_window_set_decorated (window, FALSE);
    gtk_window_set_resizable (window, FALSE);
    gtk_window_stick (window);
    //gtk_window_set_icon_name (window, CLOCK_ICON);
    gtk_window_set_default_size (GTK_WINDOW (window),
                                 200, 200);
}

GtkWidget *
menu_window_new (void)
{
    MenuWindow *menuwin;

    menuwin = g_object_new (MENU_TYPE_WINDOW,
                   "type", GTK_WINDOW_TOPLEVEL,
                   NULL);

    return GTK_WIDGET (menuwin);
}

