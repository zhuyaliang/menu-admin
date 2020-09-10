#include "app-list.h"
#include "app-menu.h"

static void list_view_init(GtkWidget *list)
{
    GtkCellRenderer   *renderer_icon,*renderer_text;
    GtkTreeViewColumn *column;
    column=gtk_tree_view_column_new ();

    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                     GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 280);
    gtk_tree_view_column_set_spacing (GTK_TREE_VIEW_COLUMN (column),8);

    renderer_icon = gtk_cell_renderer_pixbuf_new();   //user icon
    g_object_set (G_OBJECT(renderer_icon),"stock-size",5); 
    gtk_tree_view_column_pack_start (column, renderer_icon, FALSE);
    gtk_tree_view_column_set_attributes (column,
                                         renderer_icon,
                                         "gicon",
                                         COL_USER_FACE,
                                         NULL);

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
void refresh_app_list_data(GtkWidget   *list,
                           const gchar *app_name,
                           GIcon       *icon,
                           gpointer     data)
{
    GtkListStore *store;
    GtkTreeIter   iter;
    char         *label;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    label =  g_markup_printf_escaped("<span color = \'grey\' size='large' weight='bold'>%s</span>", app_name);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, 
                       &iter,
                       COL_USER_FACE, icon,  //icon
                       LIST_DATA, data,  
                       LIST_LABEL,label,     //two name
                       -1);
    g_free (label);
}

GtkWidget *create_empty_app_list (GtkListStore *store)
{   
    GtkWidget        *list;
    
    list= gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    list_view_init (list);
  
    return list;
}
GtkListStore *create_store (void)
{
    GtkListStore     *store;
    store = gtk_list_store_new(N_COLUMNS,
                               G_TYPE_ICON,
                               G_TYPE_STRING,
                               G_TYPE_POINTER);

    return store;
}    
