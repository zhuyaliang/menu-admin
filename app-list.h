#ifndef __APP_LIST__
#define __APP_LIST__

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "app-menu.h"

enum
{   
    COL_USER_FACE= 0,
    LIST_LABEL ,
    LIST_DATA ,
    N_COLUMNS
};
GtkWidget     *create_empty_app_list  (GtkListStore *store);

GtkListStore  *create_store           (void);

void          refresh_app_list_data   (GtkWidget   *list,
                                       const gchar *app_name,
                                       GIcon       *icon,
                                       gpointer     data);

#endif
