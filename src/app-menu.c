#define MATEMENU_I_KNOW_THIS_IS_UNSTABLE
#include <matemenu-tree.h>
#include "app-menu.h"
#include "app-util.h"
 

#define  MENU_ADMID_SCHEMA                 "org.admin.menu"
#define  MENU_DEFAULT_ITEM                 "default-item"
#define  MENU_ICON_SIZE                    "icon-size"
#define  MENU_FONT_SIZE                    "font-size"

#define _(STRING)  gettext(STRING)
struct _AppMenu
{
    GObject       parent;
    GtkContainer *container;
    GtkBox       *box;    
    GtkWidget    *category_tree;
    GtkWidget    *subapp_tree;
    GtkListStore *category_store;
    GtkListStore *subapp_store;
    GSettings    *settings;
    char         *default_item;
    gint          icon_size;
    gint          font_size;
};
struct _AppMenuClass
{
    GObjectClass parent_class;
    void (*add_menu) (AppMenu *app_menu);
    void (*delete_menu) (AppMenu *app_menu);
};
G_DEFINE_TYPE (AppMenu, app_menu, G_TYPE_OBJECT)

enum {
  SHOW_MENU,
  ADD_MENU,
  CHANGED_MENU,
  LAST_SIGNAL,
};

enum
{   
    COL_USER_FACE= 0,
    LIST_LABEL ,
    LIST_DATA ,
    N_COLUMNS
};
typedef enum
{
    ELLIPSIZE_NONE,
    ELLIPSIZE_START,
    ELLIPSIZE_MIDDLE,
    ELLIPSIZE_END
} EllipsizeMode;

static char *font_size [] = {"xx-small","x-small","small","medium","large","x-large","xx-large"};
static guint signals[LAST_SIGNAL] = { 0 };

static void submenu_to_display (AppMenu *menu);
static gboolean submenu_to_display_in_idle (gpointer data);
typedef char * (*LookupInDir) (const char *basename, const char *dir);

static void list_view_init(GtkWidget *list,int renderer_size,guint icon_size)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer   *renderer_icon,*renderer_text;
    column=gtk_tree_view_column_new ();

    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                     GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), renderer_size);
    gtk_tree_view_column_set_spacing (GTK_TREE_VIEW_COLUMN (column),8);
    g_object_set_data (G_OBJECT (list),
                       "tree-list-column",
                       column);

    renderer_icon = gtk_cell_renderer_pixbuf_new();   //user icon
    g_object_set (G_OBJECT(renderer_icon),"stock-size",icon_size,NULL);
    gtk_tree_view_column_pack_start (column, renderer_icon, FALSE);
    gtk_tree_view_column_set_attributes (column,
                                         renderer_icon,
                                         "gicon",
                                         COL_USER_FACE,
                                         NULL);
    g_object_set_data (G_OBJECT (list),
                       "tree-list-renderer-icon",
                       renderer_icon);

    gtk_cell_renderer_set_fixed_size (renderer_icon,48,48);    
    renderer_text = gtk_cell_renderer_text_new();     //user real name text
    gtk_tree_view_column_pack_start(column,renderer_text,FALSE);
    gtk_tree_view_column_add_attribute(column,
                                       renderer_text,
                                       "markup",
                                       LIST_LABEL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(list),FALSE);
}
static char *get_ellipsize_app_name (const char *str,int length,EllipsizeMode mode)
{
    int len,start_width,end_width;
    const char *text = "...";
    char  *ellipsize,*start_str,*end_str;
    char  *ret;

    len = g_utf8_strlen (str,-1);
    if (len <= length)
    {
        return g_strdup (str);
    }
    switch (mode)
    {
        case ELLIPSIZE_START:
            ellipsize = g_utf8_substring (str,len - length,len);
            ret = g_strdup_printf ("%s%s",text,ellipsize);
            g_free (ellipsize);
            break;
        case ELLIPSIZE_MIDDLE:
            start_width = length/2;
            end_width = length - length/2;
            start_str = g_utf8_substring (str,0,start_width);
            end_str = g_utf8_substring (str,len - end_width,len);
            ret = g_strdup_printf ("%s%s%s",start_str,text,end_str);
            g_free (start_str);
            g_free (end_str);
            break;
        case ELLIPSIZE_END:
            ellipsize = g_utf8_substring (str,0,length);
            ret = g_strdup_printf ("%s%s",ellipsize,text);
            g_free (ellipsize);
        default:
            break;
    }
    return ret;
}
static void refresh_app_list_data(GtkWidget   *list,
                                  const gchar *app_name,
                                  GIcon       *icon,
                                  char        *default_item,
                                  gint         index,
                                  gpointer     data)
{
    GtkListStore *store;
    GtkTreeIter   iter;
    char   *ellipsize;
    char         *label;
    GtkTreeSelection *selection;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    ellipsize = get_ellipsize_app_name (app_name,14,ELLIPSIZE_MIDDLE);
    label =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>",font_size[index],ellipsize);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, 
                       &iter,
                       COL_USER_FACE, icon,  //icon
                       LIST_DATA, data,  
                       LIST_LABEL,label,     //two name
                       -1);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(list));
    if (default_item != NULL)
    {
        if (g_strcmp0 (app_name,default_item) == 0 )
            gtk_tree_selection_select_iter (selection,&iter);
    }
    g_free (label);
    g_free (ellipsize);
}

static GtkWidget *create_empty_app_list (GtkListStore *store,
                                         int           renderer_size,
                                         int           icon_size)
{   
    GtkWidget        *list;
    
    list= gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    list_view_init (list,renderer_size,icon_size);
  
    return list;
}
static GtkListStore *create_store (void)
{
    GtkListStore     *store;
    store = gtk_list_store_new(N_COLUMNS,
                               G_TYPE_ICON,
                               G_TYPE_STRING,
                               G_TYPE_POINTER);

    return store;
}

static GtkWidget *
create_submenu_entry (AppMenu               *menu,
                      MateMenuTreeDirectory *directory)
{
    refresh_app_list_data (menu->category_tree,
                           matemenu_tree_directory_get_name (directory),
                           matemenu_tree_directory_get_icon (directory),
                           menu->default_item,
                           menu->font_size,
                          (gpointer)directory);


    return menu->category_tree;
}

static void
create_submenu (AppMenu          *menu,
                MateMenuTreeDirectory *directory,
                MateMenuTreeDirectory *alias_directory)
{
    
    GtkWidget *menuitem;

    if (alias_directory)
        menuitem = create_submenu_entry (menu, alias_directory);
    else
        menuitem = create_submenu_entry (menu, directory);

}

static GtkWidget *
create_item_context_menu (GtkWidget   *item)
{
    //MateMenuTreeEntry     *entry;
    //MateMenuTreeDirectory *directory;
    //MateMenuTree          *tree;
    GtkWidget          *menu;
    GtkWidget          *submenu;
    GtkWidget          *menuitem;
    //const char         *menu_filename;

    menu = gtk_menu_new ();
    g_signal_connect (item, "destroy",
              G_CALLBACK (gtk_widget_destroy), menu);

    menuitem = gtk_menu_item_new_with_mnemonic (_("Add this launcher to _panel"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);
    
    menuitem = gtk_menu_item_new_with_mnemonic (_("Add this launcher to _desktop"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);


    submenu = gtk_menu_new ();

    menuitem = gtk_menu_item_new_with_mnemonic (_("_Entire menu"));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    menuitem = gtk_menu_item_new_with_mnemonic (_("Add this as _drawer to panel"));
    gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
    gtk_widget_show (menuitem);

    menuitem = gtk_menu_item_new_with_mnemonic (_("Add this as _menu to panel"));
    gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
    gtk_widget_show (menuitem);

    return menu;
}

static gboolean
show_item_menu (GtkWidget      *item,
                GdkEvent *event)
{
    GtkWidget   *menu;
    GtkWidget   *window;
    GtkWidget   *toplevel;
    GdkScreen   *screen;
    GdkVisual   *visual;
    GtkStyleContext *context;

    menu = create_item_context_menu (item);
    window  = gtk_widget_get_toplevel (GTK_WIDGET (item));
    gtk_menu_set_screen (GTK_MENU (menu),gtk_window_get_screen (GTK_WINDOW (window)));
    /* Set up theme and transparency support */
    toplevel = gtk_widget_get_toplevel (menu);
    /* Fix any failures of compiz/other wm's to communicate with gtk for transparency */
    screen = gtk_widget_get_screen (GTK_WIDGET (toplevel));
    visual = gdk_screen_get_rgba_visual (screen);
    gtk_widget_set_visual(GTK_WIDGET (toplevel), visual);
    /* Set menu and it's toplevel window to follow panel theme */
    context = gtk_widget_get_style_context (GTK_WIDGET (toplevel));
    gtk_style_context_add_class(context,"gnome-panel-menu-bar");
    gtk_style_context_add_class(context,"mate-panel-menu-bar");
    gtk_menu_popup_at_pointer (GTK_MENU (menu), event);

    return TRUE;
}

static gboolean
menuitem_button_press_event (GtkWidget      *menuitem,
                             GdkEventButton *event)
{
    if (event->button == 3)
        return show_item_menu (menuitem, (GdkEvent *) event);

    return FALSE;
}
static void
grab_widget (GtkWidget *widget)
{

    GdkWindow *window;
    GdkDisplay *display;
    GdkSeat *seat;

    g_return_if_fail (widget != NULL);
    window = gtk_widget_get_window (widget);
    display = gdk_window_get_display (window);

    seat = gdk_display_get_default_seat (display);
    gdk_seat_grab (seat, window,
                   GDK_SEAT_CAPABILITY_ALL, TRUE,
                   NULL, NULL, NULL, NULL);
    gtk_widget_hide (widget);
}
static void
drag_data_get_menu_cb (GtkWidget        *widget,
                       GdkDragContext   *context,
                       GtkSelectionData *selection_data,
                       guint             info,
                       guint             time,
                       AppMenu          *menu)
{
    const char *path;
    char       *uri;
    char       *uri_list;
    MateMenuTreeEntry *entry;

    entry = g_object_get_data (G_OBJECT (menu),"panel-menu-tree-entry");
    path = matemenu_tree_entry_get_desktop_file_path (entry);
    uri = g_filename_to_uri (path, NULL, NULL);
    uri_list = g_strconcat (uri, "\r\n", NULL);
    g_free (uri);

    gtk_selection_data_set (selection_data,
                gtk_selection_data_get_target (selection_data), 8, (guchar *)uri_list,
                strlen (uri_list));
    g_free (uri_list);
}
static void
drag_begin_menu_cb (GtkWidget *widget, GdkDragContext     *context)
{
    /* FIXME: workaround for a possible gtk+ bug
     *    See bugs #92085(gtk+) and #91184(panel) for details.
     *    Maybe it's not needed with GtkTooltip?
     */
    g_object_set (widget, "has-tooltip", FALSE, NULL);
}
static void
drag_end_menu_cb (GtkWidget *widget, GdkDragContext     *context)
{
    GtkWidget *xgrab_shell;
    GtkWidget *parent;

    parent = gtk_widget_get_parent (widget);
    xgrab_shell = NULL;
    g_object_set (widget, "has-tooltip", TRUE, NULL);
    while (parent)
    {
        gboolean viewable = TRUE;
        GtkWidget *tmp = parent;
        while (tmp)
        {
            if (!gtk_widget_get_mapped (tmp))
            {
                viewable = FALSE;
                    break;
            }
            tmp = gtk_widget_get_parent (tmp);
        }

        if (viewable)
            xgrab_shell = parent;
        parent = gtk_widget_get_parent (parent);
    }
    if (xgrab_shell)
    {
        grab_widget (xgrab_shell);
     //   grab_widget (gtk_widget_get_parent(widget));
    } 
}

static void
create_menuitem (AppMenu               *menu,
                 MateMenuTreeEntry     *entry,
                 MateMenuTreeDirectory *alias_directory)
{
    GDesktopAppInfo *ginfo;
    //const gchar* app_name;

    ginfo = matemenu_tree_entry_get_app_info (entry);
    //desc= g_app_info_get_description(G_APP_INFO(ginfo));
    //gename = g_desktop_app_info_get_generic_name(ginfo);

    refresh_app_list_data (menu->subapp_tree,
                           g_app_info_get_name(G_APP_INFO(ginfo)),
                           g_app_info_get_icon(G_APP_INFO(ginfo)),
                           NULL,
                           menu->font_size,
                           (gpointer)entry);
}

static AppMenu *
populate_menu_from_directory (AppMenu               *menu,
                              MateMenuTreeDirectory *directory)
{
    //GList    *children;
    //gboolean  add_separator;
    MateMenuTreeIter *iter;
    MateMenuTreeItemType type;

    iter = matemenu_tree_directory_iter (directory);
    gtk_list_store_clear (menu->subapp_store);
    while ((type = matemenu_tree_iter_next (iter)) != MATEMENU_TREE_ITEM_INVALID)
    {
        gpointer item;
        switch (type)
        {
            case MATEMENU_TREE_ITEM_DIRECTORY:
                item = matemenu_tree_iter_get_directory(iter);
                create_submenu (menu, item, NULL);
                matemenu_tree_item_unref (item);
                break;

            case MATEMENU_TREE_ITEM_ENTRY:
                item = matemenu_tree_iter_get_entry (iter);
                create_menuitem (menu, item, NULL);
                matemenu_tree_item_unref (item);
                break;

            case MATEMENU_TREE_ITEM_SEPARATOR :
                /* already added */
                break;

            case MATEMENU_TREE_ITEM_ALIAS:
                g_print ("MATEMENU_TREE_ITEM_ALIAS \r\n");
                item = matemenu_tree_iter_get_alias(iter);
            //    create_menuitem_from_alias (menu, item);
                matemenu_tree_item_unref (item);
                break;

            case MATEMENU_TREE_ITEM_HEADER:
                g_print ("MATEMENU_TREE_ITEM_HEADER \r\n");
                item = matemenu_tree_iter_get_header(iter);
              //  create_header (menu, item);
                matemenu_tree_item_unref (item);
                break;

            default:
                break;
        }
    }
    matemenu_tree_iter_unref (iter);

    return menu;
}

static void
submenu_to_display (AppMenu *menu)
{
    MateMenuTree           *tree;
    MateMenuTreeDirectory  *directory;
    const char             *menu_path;
/*
    if (!g_object_get_data (G_OBJECT (menu), "panel-menu-needs-loading"))
        return;
*/
    g_object_set_data (G_OBJECT (menu), "panel-menu-needs-loading", NULL);
    directory = g_object_get_data (G_OBJECT (menu),"panel-menu-tree-directory");
    if (!directory) 
    {
        menu_path = g_object_get_data (G_OBJECT (menu),"panel-menu-tree-path");
        if (!menu_path)
            return;
        tree = g_object_get_data (G_OBJECT (menu), "panel-menu-tree");
        if (!tree)
            return;
        directory = matemenu_tree_get_directory_from_path (tree,
                                                           menu_path);
        g_object_set_data_full (G_OBJECT (menu),
                                "panel-menu-tree-directory",
                                directory,
                                (GDestroyNotify) matemenu_tree_item_unref);
    }
    if (directory)
        populate_menu_from_directory (menu, directory);
}


static gboolean
submenu_to_display_in_idle (gpointer data)
{
    AppMenu *menu = APP_MENU (data);
    g_object_set_data (G_OBJECT (menu), "panel-menu-idle-id", NULL);

    submenu_to_display (menu);

    return FALSE;
}
 static void
 remove_submenu_to_display_idle (gpointer data)
 {
     guint idle_id = GPOINTER_TO_UINT (data);
 
     g_source_remove (idle_id);
 }
static void view_submenu(GtkWidget *widget,  gpointer data)
{
    AppMenu      *menu = APP_MENU(data);
    GtkTreeIter   iter;
    GtkTreeModel *model;
    const char   *item_name;

    MateMenuTreeDirectory *directory;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter))
    {
        gtk_tree_model_get (model, &iter,
                            LIST_DATA, &directory,
                            -1);
        item_name = matemenu_tree_directory_get_name (directory);
        g_settings_set_string (menu->settings,MENU_DEFAULT_ITEM,item_name);
        gtk_widget_show (menu->subapp_tree);
        g_signal_emit (menu, signals[SHOW_MENU], 0, directory,NULL);
    }

}
static void create_category_tree (AppMenu *menu)
{
    GtkTreeSelection *selection;
    //GtkTreeModel      *model;
   
    menu->category_store = create_store ();
    menu->category_tree = create_empty_app_list (menu->category_store,210,menu->icon_size);
    gtk_tree_view_set_hover_selection (GTK_TREE_VIEW(menu->category_tree),TRUE);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(menu->category_tree));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    //model=gtk_tree_view_get_model(GTK_TREE_VIEW(menu->category_tree));
    g_signal_connect(selection, 
                    "changed", 
                     G_CALLBACK(view_submenu), 
                     menu);
    gtk_box_pack_start (menu->box, menu->category_tree, FALSE, FALSE, 0);
    gtk_widget_show (menu->category_tree);
}
static char *
_lookup_in_applications_subdir (const char *basename,
                const char *dir)
{
    char *path;

    path = g_build_filename (dir, "applications", basename, NULL);
    if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_free (path);
        return NULL;
    }

    return path;
}

static char *
_app_g_lookup_in_data_dirs_internal (const char *basename,
                       LookupInDir lookup)
{
    const char * const *system_data_dirs;
    const char          *user_data_dir;
    char                *retval;
    int                  i;

    user_data_dir    = g_get_user_data_dir ();
    system_data_dirs = g_get_system_data_dirs ();

    if ((retval = lookup (basename, user_data_dir)))
        return retval;

    for (i = 0; system_data_dirs[i]; i++)
        if ((retval = lookup (basename, system_data_dirs[i])))
            return retval;

    return NULL;
}
static char *
app_g_lookup_in_applications_dirs (const char *basename)
{
    return _app_g_lookup_in_data_dirs_internal (basename,
                              _lookup_in_applications_subdir);
}
static GtkWidget *
panel_error_dialog (GtkWindow  *parent,
            GdkScreen  *screen,
            const char *dialog_class,
            gboolean    auto_destroy,
            const char *primary_text,
            const char *secondary_text)
{
    GtkWidget *dialog;
    char      *freeme;

    freeme = NULL;

    if (primary_text == NULL) {
        g_warning ("NULL dialog");
         /* No need to translate this, this should NEVER happen */
        freeme = g_strdup_printf ("Error with displaying error "
                      "for dialog of class %s",
                      dialog_class);
        primary_text = freeme;
    }

    dialog = gtk_message_dialog_new (parent, 0, GTK_MESSAGE_ERROR,
                     GTK_BUTTONS_CLOSE, "%s", primary_text);
    if (secondary_text != NULL)
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                              "%s", secondary_text);

    if (screen)
        gtk_window_set_screen (GTK_WINDOW (dialog), screen);

    if (!parent) {
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), FALSE);
        /* FIXME: We need a title in this case, but we don't know what
         * the format should be. Let's put something simple until
         * the following bug gets fixed:
         * http://bugzilla.gnome.org/show_bug.cgi?id=165132 */
        gtk_window_set_title (GTK_WINDOW (dialog), "Error");
    }

    gtk_widget_show_all (dialog);

    if (auto_destroy)
        g_signal_connect_swapped (G_OBJECT (dialog), "response",
                      G_CALLBACK (gtk_widget_destroy),
                      G_OBJECT (dialog));

    if (freeme)
        g_free (freeme);

    return dialog;
}
static void
_app_launch_error_dialog (const gchar *name,
                GdkScreen   *screen,
                const gchar *message)
{
    char *primary;

    if (name)
        primary = g_markup_printf_escaped ("Could not launch '%s'",
                           name);
    else
        primary = g_strdup ("Could not launch application");

    panel_error_dialog (NULL, screen, "cannot_launch", TRUE,
                primary, message);
    g_free (primary);
}

static gboolean
_app_launch_handle_error (const gchar  *name,
                GdkScreen    *screen,
                GError       *local_error,
                GError      **error)
{
    if (g_error_matches (local_error,
                 G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
        g_error_free (local_error);
        return TRUE;
    }

    else if (error != NULL)
        g_propagate_error (error, local_error);

    else {
        _app_launch_error_dialog (name, screen, local_error->message);
        g_error_free (local_error);
    }

    return FALSE;
}
static void
dummy_child_watch (GPid     pid,
           gint     status,
           gpointer user_data)
{   
    /* Nothing, this is just to ensure we don't double fork
     * and break pkexec:
     * https://bugzilla.gnome.org/show_bug.cgi?id=675789
     */
}

static void
gather_pid_callback (GDesktopAppInfo   *gapp,
             GPid               pid,
             gpointer           data)
{
    g_child_watch_add (pid, dummy_child_watch, NULL);
}

static gboolean
app_app_info_launch_uris (GDesktopAppInfo   *appinfo,
                GList      *uris,
                GdkScreen  *screen,
                const gchar *action,
                guint32     timestamp,
                GError    **error)
{
    GdkAppLaunchContext *context;
    GError              *local_error;
    gboolean             retval;
    GdkDisplay          *display;

    g_return_val_if_fail (G_IS_DESKTOP_APP_INFO (appinfo), FALSE);
    g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    display = gdk_display_get_default ();
    context = gdk_display_get_app_launch_context (display);
    gdk_app_launch_context_set_screen (context, screen);
    gdk_app_launch_context_set_timestamp (context, timestamp);

    local_error = NULL;
    if (action == NULL) {
        retval = g_desktop_app_info_launch_uris_as_manager (appinfo, uris,
                           G_APP_LAUNCH_CONTEXT (context),
                           G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                           NULL, NULL, gather_pid_callback, appinfo,
                           &local_error);
    } else {
        g_desktop_app_info_launch_action (appinfo, action, G_APP_LAUNCH_CONTEXT (context));
        retval = TRUE;
    }

    g_object_unref (context);

    if ((local_error == NULL) && (retval == TRUE))
        return TRUE;

    return _app_launch_handle_error (g_app_info_get_name (G_APP_INFO(appinfo)),
                       screen, local_error, error);
}

static gboolean
app_launch_desktop_file (const char  *desktop_file,
               GdkScreen   *screen,
               GError     **error)
{            
    GDesktopAppInfo *appinfo;
    gboolean         retval;

    g_return_val_if_fail (desktop_file != NULL, FALSE);
    g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    appinfo = NULL;

    if (g_path_is_absolute (desktop_file))
        appinfo = g_desktop_app_info_new_from_filename (desktop_file);
    else {
        char *full;

        full = app_g_lookup_in_applications_dirs (desktop_file);
        if (full) {
            appinfo = g_desktop_app_info_new_from_filename (full);
            g_free (full);
        }
    }

    if (appinfo == NULL)
        return FALSE;

    retval = app_app_info_launch_uris (appinfo, NULL, screen, NULL,
                         gtk_get_current_event_time (),
                         error);

    g_object_unref (appinfo);

    return retval;
}

static void activate_app_def (GtkTreeView *treeview,
                              GtkTreePath *path,
                              GtkTreeViewColumn *col,
                              gpointer userdata)
{
    GtkTreeModel      *model;
    GtkTreeIter        iter;
    MateMenuTreeEntry *entry;
    const char        *file_path;
    GdkScreen         *screen;
    GtkWidget         *window;

    model = gtk_tree_view_get_model(treeview);
    window  = gtk_widget_get_toplevel (GTK_WIDGET (treeview));
    screen = gtk_window_get_screen (GTK_WINDOW(window));

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter, LIST_DATA, &entry, -1);
        g_object_set_data_full (G_OBJECT (treeview),
                               "panel-menu-tree-entry",
                                matemenu_tree_item_ref (entry),
                               (GDestroyNotify) matemenu_tree_item_unref);
        file_path = matemenu_tree_entry_get_desktop_file_path (entry);
        app_launch_desktop_file (file_path, screen, NULL);
    }
}
static void switch_subapp (GtkWidget *widget,  gpointer data)
{
    AppMenu      *menu = APP_MENU(data);
    GtkTreeIter   iter;
    GtkTreeModel *model;
    GIcon        *gicon;
    MateMenuTreeEntry *entry;
    GDesktopAppInfo   *ginfo;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter))
    {
        gtk_tree_model_get (model, &iter,
                            LIST_DATA, &entry,
                            -1);
        g_object_set_data_full (G_OBJECT (menu),
                               "panel-menu-tree-entry",
                                matemenu_tree_item_ref (entry),
                               (GDestroyNotify) matemenu_tree_item_unref);

        ginfo = matemenu_tree_entry_get_app_info (entry);
        if (g_app_info_get_icon (G_APP_INFO(ginfo)) != NULL) 
        {
            gicon = g_app_info_get_icon (G_APP_INFO(ginfo));
            if (gicon != NULL) 
            {
                gtk_drag_source_set_icon_gicon (menu->subapp_tree, gicon);
            }
        }
        menu_util_set_tooltip_text (menu->subapp_tree,g_app_info_get_name(G_APP_INFO(ginfo)));
    }
}
static void create_subapp_tree (AppMenu *menu)
{
    //GtkTreeModel      *model;
    GtkTreeSelection *selection;
    static GtkTargetEntry menu_item_targets[] = {
             { "text/uri-list", 0, 0 }
         };
   
    menu->subapp_store = create_store ();
    menu->subapp_tree = create_empty_app_list (menu->subapp_store,270,menu->icon_size);
    gtk_tree_view_set_hover_selection (GTK_TREE_VIEW(menu->subapp_tree),TRUE);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(menu->subapp_tree));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    //model=gtk_tree_view_get_model(GTK_TREE_VIEW(menu->subapp_tree));

    gtk_tree_view_set_activate_on_single_click (GTK_TREE_VIEW(menu->subapp_tree),TRUE);
    gtk_container_add (menu->container, menu->subapp_tree);
    gtk_drag_source_set (menu->subapp_tree,
                         GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
                         menu_item_targets, 1,
                         GDK_ACTION_COPY);
 
    g_signal_connect (menu->subapp_tree, 
                     "row-activated",
                      G_CALLBACK (activate_app_def), 
                      menu);

    g_signal_connect (menu->subapp_tree, 
                     "button_press_event",
                      G_CALLBACK (menuitem_button_press_event), 
                      menu);
  
    g_signal_connect (selection,
                     "changed",
                      G_CALLBACK(switch_subapp),
                      menu);
    
    g_signal_connect (G_OBJECT (menu->subapp_tree), 
                     "drag_begin",
                      G_CALLBACK (drag_begin_menu_cb), 
                      NULL);

    g_signal_connect (menu->subapp_tree, 
                     "drag_data_get",
                      G_CALLBACK (drag_data_get_menu_cb), 
                      menu);

    g_signal_connect (menu->subapp_tree,
                     "drag_end",
                      G_CALLBACK (drag_end_menu_cb),
                      NULL);

}
static void
handle_matemenu_tree_changed (MateMenuTree *tree,AppMenu *menu)
{
    GError *error = NULL;
    guint idle_id;
    
    gtk_list_store_clear (menu->subapp_store);
    gtk_list_store_clear (menu->category_store);
    if (! matemenu_tree_load_sync (tree, &error)) {
        g_warning("Menu tree reload got error:%s\n", error->message);
        g_error_free(error);
    }

    g_object_set_data_full (G_OBJECT (menu),
                "panel-menu-tree-directory",
                NULL, NULL);

    g_object_set_data (G_OBJECT (menu),
               "panel-menu-needs-loading",
               GUINT_TO_POINTER (TRUE));

    idle_id = g_idle_add_full (G_PRIORITY_LOW,
                   submenu_to_display_in_idle,
                   menu,
                   NULL);
    g_object_set_data_full (G_OBJECT (menu),
                "panel-menu-idle-id",
                GUINT_TO_POINTER (idle_id),
                remove_submenu_to_display_idle);
}
static void
app_menu_changed (AppMenu       *menu,
                  guint         icon_size,
                  MateMenuTree  *tree)
{
    GtkCellRenderer   *renderer_icon;

    renderer_icon = g_object_get_data (G_OBJECT (menu->category_tree),"tree-list-renderer-icon");
    g_object_set (G_OBJECT(renderer_icon),"stock-size",icon_size,NULL);

    renderer_icon = g_object_get_data (G_OBJECT (menu->subapp_tree),"tree-list-renderer-icon");
    g_object_set (G_OBJECT(renderer_icon),"stock-size",icon_size,NULL);

    handle_matemenu_tree_changed (tree,menu);
}
static void
app_menu_init (AppMenu *self)
{
    self->settings = g_settings_new (MENU_ADMID_SCHEMA);
    self->default_item = g_settings_get_string (self->settings,MENU_DEFAULT_ITEM);
    self->icon_size = g_settings_get_enum (self->settings, MENU_ICON_SIZE);
    self->font_size = g_settings_get_enum (self->settings, MENU_FONT_SIZE);
}
static void
app_menu_finalize (GObject *object)
{
    AppMenu *self = APP_MENU (object);
    if (self->settings)
        g_object_unref (self->settings);
    self->settings = NULL;
}
static void
app_menu_class_init (AppMenuClass *klass)
{
    GObjectClass   *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = app_menu_finalize;

    signals[SHOW_MENU] =
    g_signal_new ("show-menu",
                  APP_TYPE_MENU,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

    signals[ADD_MENU] =
    g_signal_new ("add_menu",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

    signals[CHANGED_MENU] =
    g_signal_new ("changed_menu",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  G_TYPE_UINT);

}
static void gsettings_icon_size_changed (GSettings *settings,
                                         gchar     *key,
                                         AppMenu   *menu)
{
    menu->icon_size = g_settings_get_enum (settings,key);
    g_signal_emit (menu, signals[CHANGED_MENU], 0, menu->icon_size, NULL);
}
static void gsettings_font_size_changed (GSettings *settings,
                                         gchar     *key,
                                         AppMenu   *menu)
{
    menu->font_size = g_settings_get_enum (settings,key);
    g_signal_emit (menu, signals[CHANGED_MENU], 0, menu->icon_size, NULL);
}
static void show_submenu (AppMenu *menu,MateMenuTreeDirectory *directory,gpointer data)
{
    populate_menu_from_directory (menu,directory);
}
AppMenu *app_menu_new (void)
{
    AppMenu *menu;

    menu = g_object_new (APP_TYPE_MENU,NULL);

    g_signal_connect (menu,
                     "show-menu",
                      G_CALLBACK (show_submenu),
                      NULL);

    g_signal_connect (menu->settings,
                      "changed::" MENU_ICON_SIZE,
                      G_CALLBACK (gsettings_icon_size_changed),
                      menu);

    g_signal_connect (menu->settings,
                      "changed::" MENU_FONT_SIZE,
                      G_CALLBACK (gsettings_font_size_changed),
                      menu);
    return menu;
}
AppMenu *
create_applications_menu (const char   *menu_file,
                          GtkContainer *container,
                          GtkBox       *box)
{
    MateMenuTree *tree;
    AppMenu      *menu;
    guint        idle_id;
    GError       *error = NULL;

    menu = app_menu_new();
    menu->container = container;
    menu->box = box;
    create_category_tree (menu);
    create_subapp_tree (menu);
    
    tree = matemenu_tree_new (menu_file, MATEMENU_TREE_FLAGS_SORT_DISPLAY_NAME);
    if (! matemenu_tree_load_sync (tree, &error)) 
    {
        g_warning("Menu tree loading got error:%s\n", error->message);
        g_error_free(error);
        g_object_unref(tree);
        tree = NULL;
    }

    g_object_set_data_full (G_OBJECT (menu),
                "panel-menu-tree",
                g_object_ref(tree),
                (GDestroyNotify) g_object_unref);

    g_object_set_data_full (G_OBJECT (menu),
                "panel-menu-tree-path",
                g_strdup ("/"),
                (GDestroyNotify) g_free);

    g_object_set_data (G_OBJECT (menu),
               "panel-menu-needs-loading",
               GUINT_TO_POINTER (TRUE));

    idle_id = g_idle_add_full (G_PRIORITY_LOW,
                   submenu_to_display_in_idle,
                   menu,
                   NULL);

    g_object_set_data_full (G_OBJECT (menu),
                "panel-menu-idle-id",
                GUINT_TO_POINTER (idle_id),
                remove_submenu_to_display_idle);

    g_signal_connect (tree,
                     "changed",
                      G_CALLBACK (handle_matemenu_tree_changed),
                      menu);

    g_signal_connect (menu,
                     "changed-menu",
                      G_CALLBACK (app_menu_changed),
                      tree);
    g_object_unref(tree);
    return menu;
}
