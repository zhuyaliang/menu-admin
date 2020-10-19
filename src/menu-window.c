#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "menu-window.h"
#include "app-menu.h"
#include "app-util.h"
#include "config.h"


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
menu_admin_settings (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  g_print ("Text set from normal menu item\r\n");
}

static void
menu_admin_about (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
    
    GtkWindow *parent = GTK_WINDOW (user_data);

    static const gchar* artists[] = {
         "zhuyaliang <15132211195@163.com>",
         NULL
     };
    static const gchar* authors[] = {
         "Yaliang Zhu <15132211195@163.com>",
         NULL
    };

    gtk_show_about_dialog (parent,
                           "authors", authors,
                           "artists", artists,
                           "translator-credits", _("translator-credits"),
                           "comments", _("applications menu admin"),
                           "copyright", "Copyright © 2020 zhuyaliang",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           "logo-icon-name", "org.gnome.Logs",
                           "version", PACKAGE_VERSION,
                           "website", PACKAGE_NAME, NULL);

}

static const GActionEntry actions[] = {
  { "menu-admin-about", menu_admin_about},
  { "menu-admin-settings", menu_admin_settings}
};

static GtkWidget *create_menu_button (MenuWindow *menuwin)
{
    GtkWidget  *menu_button;
    GtkWidget  *image;
    GMenuModel *menu_model;
    GtkBuilder *builder;
    GSimpleActionGroup *action_group;

    menu_button = gtk_menu_button_new ();
    image  = gtk_image_new_from_icon_name ("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (menu_button), image);
    gtk_button_set_relief (GTK_BUTTON(menu_button),GTK_RELIEF_NONE);
    
    action_group = g_simple_action_group_new (); 
    g_action_map_add_action_entries (G_ACTION_MAP (action_group),
                                     actions,
                                     G_N_ELEMENTS (actions),
                                     NULL);

    gtk_widget_insert_action_group (GTK_WIDGET(menuwin), "win", G_ACTION_GROUP (action_group));
  
    builder = gtk_builder_new_from_resource ("/org/admin/menu/menu-admin-function-manager.ui");
    menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "menu_model"));
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button), menu_model);

    return menu_button;
}
static GtkWidget *create_manager_menu (MenuWindow *menuwin)
{
    GtkWidget *table;
    GtkWidget *search_button;
    GtkWidget *menu_button;
    GtkWidget *user_button;
    GtkWidget *separator;

    table = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(table),TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);

    separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(table) ,separator, 0, 0, 3, 1);

    search_button = set_button_style ("edit-find-symbolic");
    gtk_button_set_relief (GTK_BUTTON(search_button),GTK_RELIEF_NONE);
    gtk_grid_attach(GTK_GRID(table), search_button, 0, 1, 1, 1);

    menu_button = create_menu_button (menuwin);
    
    gtk_grid_attach(GTK_GRID(table), menu_button, 1, 1, 1, 1);

    user_button = set_button_style ("avatar-default-symbolic");
    gtk_button_set_relief (GTK_BUTTON(user_button),GTK_RELIEF_NONE);
    gtk_grid_attach(GTK_GRID(table), user_button, 2, 1, 1, 1);

    return table;
}
static void
menu_window_fill (MenuWindow *menuwin)
{
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *category_box,*app_vbox;
    GtkWidget *toplevel;
    GdkScreen *screen;
    GdkVisual *visual;
    GtkWidget *scroll;
    GtkWidget *table;

    set_box_background (GTK_WIDGET (menuwin));
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
    gtk_container_add (GTK_CONTAINER (menuwin), frame);

    hbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox);

    vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    
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
    table = create_manager_menu (menuwin);
    gtk_box_pack_start(GTK_BOX(hbox),table, TRUE, TRUE,0);

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
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

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

