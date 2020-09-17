#include <gtk/gtk.h>
#include "menu-window.h"

int main(int argc,char *argv[])
{
    GtkWidget *window;

    gtk_init (&argc,&argv);
    window = menu_window_new ();
    gtk_widget_show_all (window);
    gtk_main();
    return 0;
}
