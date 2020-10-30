#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "menu-window.h"
#include "app-menu.h"
#include "app-util.h"
#include "menu-tool.h"
#include "config.h"


struct _MenuWindowPrivate 
{
    AppMenu    *menu;
    GtkWidget  *stack;
    GSettings  *settings;
    GtkBuilder *builder;
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

static gboolean
font_size_mapping_get (GValue   *value,
                       GVariant *variant,
                       gpointer  user_data)
{
    const char *font_style;

    font_style = g_variant_get_string (variant,NULL);
    g_value_set_string (value, font_style);

    return TRUE;
}
static GVariant *
font_size_mapping_set (const GValue       *value,
                       const GVariantType *expected_type,
                       gpointer            user_data)
{
    return g_variant_new_string (g_value_get_string (value));
}

static GtkWidget *create_style_combox (const char **style,
                                       GSettings   *settings,
                                       const char  *key)
{
    GtkWidget *combox;
    int i = 0;

    combox = gtk_combo_box_text_new ();
    while (style[i] != NULL)
    {
        gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combox), style[i], style[i]);
        i++;
    }

    g_settings_bind_with_mapping (settings,
                                  key,
                                  combox,
                                 "active-id",
                                  G_SETTINGS_BIND_DEFAULT,
                                  font_size_mapping_get,
                                  font_size_mapping_set,
                                  NULL, NULL);
    return combox;
}

static gboolean
width_size_mapping_get (GValue   *value,
                        GVariant *variant,
                        gpointer  user_data)
{
    guint width;

    width = g_variant_get_uint32 (variant);
    g_value_set_double (value, width);

    return TRUE;
}
static GVariant *
width_size_mapping_set (const GValue       *value,
                        const GVariantType *expected_type,
                        gpointer            user_data)
{
    return g_variant_new_uint32 (g_value_get_double (value));
}
static GtkWidget *create_menu_spin (GSettings  *settings,
                                    double      min,
                                    double      max,
                                    double      step,
                                    const char *key)
{
    GtkWidget *spin;

    spin = gtk_spin_button_new_with_range (min, max, step);
    g_settings_bind_with_mapping (settings,
                                  key,
                                  spin,
                                 "value",
                                  G_SETTINGS_BIND_DEFAULT,
                                  width_size_mapping_get,
                                  width_size_mapping_set,
                                  NULL, NULL);

    return spin;
}
static void
menu_settings_response_cb (GtkDialog *dialog,
                           gint       response_id,
                           gpointer   user_data)
{
    GSettings *settings = G_SETTINGS (user_data);

    if (response_id == GTK_RESPONSE_OK)
    {
        g_settings_reset (settings,MENU_ICON_SIZE);
        g_settings_reset (settings,MENU_FONT_SIZE);
        g_settings_reset (settings,MENU_WIDTH_SIZE);
        g_settings_reset (settings,MENU_COLUMN_SPACING);
        g_settings_reset (settings,MENU_ROW_SPACING);
    }
    else if (response_id == GTK_RESPONSE_CLOSE)
    {
        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
}
static void
menu_admin_settings (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
    GtkWindow  *parent = GTK_WINDOW (user_data);
    MenuWindow *menuwin = MENU_WINDOW (user_data);
    GtkWidget  *dialog;
    GtkWidget  *box;
    GtkWidget  *dialog_area;
    GtkWidget  *table;
    GtkWidget  *combox;
    GtkWidget  *label;
    GtkWidget  *spin;

    const char *font_style [] = {"xx-small","x-small","small","medium","large","x-large","xx-large",NULL};
    const char *icon_style [] = {"16px","24px","32px","48px",NULL};
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons (_("Setting Menu"),
                                          parent,
                                          flags,
                                          _("Restore"),
                                          GTK_RESPONSE_OK,
                                          _("Close"),
                                          GTK_RESPONSE_CLOSE,
                                          NULL);
    g_signal_connect (dialog,
                     "response",
                      G_CALLBACK (menu_settings_response_cb),
                      menuwin->priv->settings);

    gtk_window_set_deletable (GTK_WINDOW (dialog), FALSE);
    gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    dialog_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    box =  gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_box_pack_start (GTK_BOX (dialog_area), box, TRUE, TRUE, 12);

    table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_box_pack_start(GTK_BOX(box), table, TRUE, TRUE, 0);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);

    label = gtk_label_new(_("Menu Font"));
    gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
    combox = create_style_combox (font_style,
                                  menuwin->priv->settings,
                                  MENU_FONT_SIZE);
    gtk_grid_attach(GTK_GRID(table), combox, 1, 0, 2, 1);

    label = gtk_label_new(_("Menu Icon"));
    gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);
    combox = create_style_combox (icon_style,
                                  menuwin->priv->settings,
                                  MENU_ICON_SIZE);
    gtk_grid_attach(GTK_GRID(table), combox, 1, 1, 2, 1);

    /*setting menu width*/
    label = gtk_label_new(_("Menu width"));
    gtk_grid_attach(GTK_GRID(table), label, 0, 2, 1, 1);
    spin = create_menu_spin (menuwin->priv->settings,
                             220, 320, 10,
                             MENU_WIDTH_SIZE);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 2, 1, 1);

    /*settings menu column*/
    label = gtk_label_new(_("Menu column"));
    gtk_grid_attach(GTK_GRID(table), label, 0, 3, 1, 1);
    spin = create_menu_spin (menuwin->priv->settings,
                             1, 50, 1,
                             MENU_COLUMN_SPACING);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 3, 1, 1);

    /*settings menu row*/
    label = gtk_label_new(_("Menu row"));
    gtk_grid_attach(GTK_GRID(table), label, 0, 4, 1, 1);
    spin = create_menu_spin (menuwin->priv->settings,
                             1, 15, 1,
                             MENU_ROW_SPACING);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 4, 1, 1);

    gtk_widget_show_all (dialog);
}

static void
menu_admin_about (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
    GtkWindow *parent = GTK_WINDOW (user_data);

    static const gchar* artists[] = {
         "tina <15132211195@163.com>",
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
                           "logo-icon-name", "menu-admin",
                           "version", PACKAGE_VERSION,
                           "website", PACKAGE_URL, NULL);

}

static void set_recent_filter_chooser (GtkWidget *dialog,const char *name,const char *type)
{
    GtkRecentFilter  *filter;
    filter = gtk_recent_filter_new ();
    gtk_recent_filter_set_name (filter, name);
    if (g_strcmp0 (type,"*") == 0)
        gtk_recent_filter_add_pattern (filter, type);
    else
        gtk_recent_filter_add_mime_type (filter, type);
    gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (dialog), filter);
}
static void
open_select_recent_file (GtkRecentChooser *chooser)
{
    GtkRecentInfo *recent_info;
    const char    *uri;
    const char    *mime_type;
    GdkScreen     *screen;
    GError        *error = NULL;
    char          *uri_utf8;
    char          *primary;
    char          *secondary;

    screen = gtk_widget_get_screen (GTK_WIDGET (chooser));
    recent_info = gtk_recent_chooser_get_current_item (chooser);
    uri = gtk_recent_info_get_uri (recent_info);
    mime_type = gtk_recent_info_get_mime_type (recent_info);

    if (menu_show_uri_force_mime_type (screen, uri, mime_type, gtk_get_current_event_time (), &error) != TRUE)
    {
        uri_utf8 = g_filename_to_utf8 (uri, -1, NULL, NULL, NULL);
        if (error)
        {
            primary = g_strdup_printf (_("Could not open recently used document \"%s\""),
                           uri_utf8);
            menu_error_dialog (NULL, screen,
                        "cannot_open_recent_doc", TRUE,
                        primary, error->message);
            g_free (primary);
            g_error_free (error);
        }
        else
        {
            primary = g_strdup_printf (_("Could not open recently used document \"%s\""),
                           uri_utf8);
            secondary = g_strdup_printf (_("An unknown error occurred while trying to open \"%s\"."),
                             uri_utf8);
            menu_error_dialog (NULL, screen,
                        "cannot_open_recent_doc", TRUE,
                        primary, secondary);
            g_free (primary);
            g_free (secondary);
        }

        g_free (uri_utf8);
    }

    gtk_recent_info_unref (recent_info);
}
static void
recent_open_response_cb (GtkDialog *dialog,
                         gint       response_id)
{
    if (response_id == GTK_RESPONSE_OK)
    {
        open_select_recent_file (GTK_RECENT_CHOOSER (dialog));
    }
    else if (response_id == GTK_RESPONSE_CANCEL)
    {
        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
}
static void
menu_admin_recent_open (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
    GtkWindow *parent = GTK_WINDOW (user_data);
    GtkWidget *dialog;

    dialog = gtk_recent_chooser_dialog_new (_("Recent Open"),
                                            parent,
                                            _("Close"),GTK_RESPONSE_CANCEL,
                                            _("Open"),GTK_RESPONSE_OK,
                                            NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    g_signal_connect (dialog,
                     "response",
                      G_CALLBACK (recent_open_response_cb),
                      NULL);

    set_recent_filter_chooser (dialog,_("All Files"),"*");
    set_recent_filter_chooser (dialog,_("PDF Files"),"application/pdf");
    set_recent_filter_chooser (dialog,_("Image Files"),"image/*");
    set_recent_filter_chooser (dialog,_("Text Files"),"text/plain");

    gtk_widget_show_all (dialog);
}
static gboolean menu_admin_fm_open (const char *uri,GtkWidget *window)
{
    GdkScreen  *screen;
    guint32     timestamp;
    GError     *error = NULL;

    screen = gtk_widget_get_screen (GTK_WIDGET (window));
    timestamp = gtk_get_current_event_time ();

    if (g_str_has_prefix (uri, "x-nautilus-search:"))
    {
        return menu_show_fm_search_uri ("nautilus-folder-handler.desktop",
                                         screen,
                                         uri,
                                         timestamp,
                                        &error);
    }
    if (g_str_has_prefix(uri, "x-caja-search:"))
    {
        return menu_show_fm_search_uri ("caja-folder-handler.desktop",
                                         screen,
                                         uri,
                                         timestamp,
                                        &error);
    }
    gtk_show_uri_on_window (NULL, uri,timestamp, &error);

    return TRUE;
}
static void
menu_admin_computer (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
    menu_admin_fm_open ("computer://",GTK_WIDGET (user_data));
}

static void
menu_admin_network (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
    menu_admin_fm_open ("network://",GTK_WIDGET (user_data));
}
static const GActionEntry actions[] = {
  { "menu-admin-about",    menu_admin_about},
  { "menu-admin-settings", menu_admin_settings},
  { "menu-admin-computer", menu_admin_computer},
  { "menu-admin-network",  menu_admin_network},
  { "menu-admin-recent-open", menu_admin_recent_open},
  { "system-user-info",    system_user_info },
  { "system-settings",     system_settings},
  { "system-switch-user",  system_switch_user},
  { "system-log-out",      system_log_out},
  { "system-lock-screen",  system_lock_screen},
  { "system-suspend",      system_suspend},
  { "system-reboot",       system_reboot},
  { "system-shutdown",     system_shutdown},
};

static GtkWidget *create_menu_button (MenuWindow *menuwin,
                                      const char *object_id,
                                      const char *icon)
{
    GtkWidget  *menu_button;
    GtkWidget  *image;
    GtkWidget  *popover;
    GSimpleActionGroup *action_group;

    menu_button = gtk_menu_button_new ();
    image  = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (menu_button), image);
    gtk_button_set_relief (GTK_BUTTON(menu_button),GTK_RELIEF_NONE);
    action_group = g_simple_action_group_new ();
    g_action_map_add_action_entries (G_ACTION_MAP (action_group),
                                     actions,
                                     G_N_ELEMENTS (actions),
                                     menuwin);

    gtk_widget_insert_action_group (GTK_WIDGET(menuwin), "win", G_ACTION_GROUP (action_group));
    popover = (GtkWidget *)gtk_builder_get_object (menuwin->priv->builder, object_id);
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (menu_button), popover);

    return menu_button;
}
static void
search_changed_cb (GtkSearchEntry *entry,
                   MenuWindow     *menuwin)
{
    const char   *text;
    GtkListStore *search_store;

    text = gtk_entry_get_text (GTK_ENTRY (entry));
    search_store = get_menu_app_search_store (menuwin->priv->menu);
    if (search_store != NULL)
        gtk_list_store_clear (search_store);
    if (view_search_app_results (menuwin->priv->menu,text) != 0 )
        gtk_stack_set_visible_child_name (GTK_STACK (menuwin->priv->stack),"search-page");
}
static GtkWidget *create_search_bar (MenuWindow *menuwin)
{
    GtkWidget *entry;
    GtkWidget *container;
    GtkWidget *searchbar;

    entry = gtk_search_entry_new ();
    container = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign (container, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (container), entry, FALSE, FALSE, 0);

    searchbar = gtk_search_bar_new ();
    gtk_search_bar_connect_entry (GTK_SEARCH_BAR (searchbar), GTK_ENTRY (entry));
    gtk_search_bar_set_show_close_button (GTK_SEARCH_BAR (searchbar), FALSE);
    gtk_container_add (GTK_CONTAINER (searchbar), container);
    g_signal_connect (entry,
                     "search-changed",
                      G_CALLBACK (search_changed_cb),
                      menuwin);
    return searchbar;
}
static void
set_visible_child (GtkToggleButton *button, gpointer data)
{
    gboolean active;

    active = gtk_toggle_button_get_active (button);
    if (!active)
    {
        gtk_stack_set_visible_child_name (GTK_STACK (data),"menu-page");
    }
}

static GtkWidget *create_manager_menu (MenuWindow *menuwin,GtkWidget *stack)
{
    GtkWidget *table;
    GtkWidget *search_button;
    GtkWidget *menu_button;
    GtkWidget *user_button;
    GtkWidget *searchbar;
    GtkWidget *separator;

    table = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(table),TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);

    separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(table) ,separator, 0, 0, 3, 1);

    search_button = set_button_style ("edit-find-symbolic");
    gtk_widget_set_tooltip_text (search_button,_("Search for applications in the menu"));
    gtk_button_set_relief (GTK_BUTTON(search_button),GTK_RELIEF_NONE);
    gtk_grid_attach(GTK_GRID(table), search_button, 0, 1, 1, 1);

    menu_button = create_menu_button (menuwin, "popover", "open-menu-symbolic");
    gtk_widget_set_tooltip_text (menu_button,_("Click to see more features"));
    gtk_grid_attach(GTK_GRID(table), menu_button, 1, 1, 1, 1);

    user_button = create_menu_button (menuwin, "popover-system", "system");
    gtk_widget_set_tooltip_text (user_button,_("View current user information"));
    //gtk_button_set_relief (GTK_BUTTON(user_button),GTK_RELIEF_NONE);
    gtk_grid_attach(GTK_GRID(table), user_button, 2, 1, 1, 1);

    searchbar = create_search_bar (menuwin);
    gtk_grid_attach(GTK_GRID(table), searchbar, 0, 2, 3, 1);

    g_signal_connect (search_button, "toggled", (GCallback) set_visible_child,stack);
    g_object_bind_property (search_button,
                           "active",
                            searchbar,
                           "search-mode-enabled",
                            G_BINDING_BIDIRECTIONAL);

    return table;
}

static GtkWidget *create_menu_box_page (MenuWindow *menuwin)
{
    GtkWidget *vbox;
    GtkWidget *scroll;
    GtkWidget *category_box,*app_vbox;

    vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    category_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), category_box,FALSE,FALSE,0);

    app_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), app_vbox,FALSE,FALSE,0);

    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (app_vbox), scroll, TRUE, TRUE, 0);
    gtk_widget_show_all (vbox);
    menuwin->priv->menu = create_applications_menu ("mate-applications.menu",
                                                     GTK_CONTAINER(scroll),
                                                     GTK_BOX(category_box));
    return vbox;
}

static GtkWidget *create_search_box_page (MenuWindow *menuwin)
{
    GtkWidget *box;
    GtkWidget *scroll;
    GtkWidget *search_tree;

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (box), scroll, TRUE, TRUE, 0);
    search_tree = get_menu_app_search_tree (menuwin->priv->menu);
    gtk_container_add (GTK_CONTAINER (scroll), search_tree);

    gtk_widget_show_all (box);

    return box;
}
static void
menu_window_fill (MenuWindow *menuwin)
{
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *stack;
    GtkWidget *menu_box;
    GtkWidget *search_box;
    GtkWidget *toplevel;
    GdkScreen *screen;
    GdkVisual *visual;
    GtkWidget *table;

    set_box_background (GTK_WIDGET (menuwin));

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
    gtk_container_add (GTK_CONTAINER (menuwin), frame);

    hbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox);

    stack = gtk_stack_new ();
    gtk_box_pack_start (GTK_BOX (hbox), stack, FALSE, FALSE, 0);
    gtk_widget_show_all (frame);

    toplevel = gtk_widget_get_toplevel (frame);
    screen = gtk_widget_get_screen(GTK_WIDGET(toplevel));
    visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(toplevel), visual);

    menu_box = create_menu_box_page (menuwin);
    gtk_stack_add_named (GTK_STACK (stack),menu_box,"menu-page");

    search_box = create_search_box_page (menuwin);
    gtk_stack_add_named (GTK_STACK (stack),search_box,"search-page");

    table = create_manager_menu (menuwin,stack);
    gtk_box_pack_start(GTK_BOX(hbox),table, TRUE, TRUE,0);
    menuwin->priv->stack = stack;
    gtk_widget_show_all (table);
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
    menuwin->priv->settings = g_settings_new (MENU_ADMID_SCHEMA);
    menuwin->priv->builder = gtk_builder_new_from_resource ("/org/admin/menu/menu-admin-function-manager.ui");

    window = GTK_WINDOW (menuwin);
    gtk_window_set_type_hint (window, GDK_WINDOW_TYPE_HINT_MENU);
    gtk_window_set_position (window,GTK_WIN_POS_MOUSE);
    gtk_window_set_decorated (window, FALSE);
    gtk_window_set_resizable (window, FALSE);
    gtk_window_stick (window);
    gtk_window_set_keep_above (window,TRUE);
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

