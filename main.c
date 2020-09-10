#include <gtk/gtk.h>
#include "app-menu.h"

int main(int argc,char *argv[])
{
    GtkWidget *window,*vbox;
    GtkWidget *category_box,*vbox2;
    GtkWidget *list1,*frame,*image; 
    GtkWidget     *scroll;
    AppMenu       *menu;

    gtk_init (&argc,&argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "List Box");
    gtk_window_set_default_size (GTK_WINDOW (window),
                                 300, 300);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_widget_destroyed),
                      &window);
    gtk_window_set_decorated (GTK_WINDOW (window),FALSE);
    vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    
    GtkWidget *toplevel = gtk_widget_get_toplevel (vbox);
    GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(toplevel));
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(toplevel), visual);
    
    category_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    //gtk_container_set_border_width (GTK_CONTAINER(category_box),10);
    gtk_box_pack_start (GTK_BOX (vbox), category_box,FALSE,FALSE,0);
    
    vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vbox2,FALSE,FALSE,0);
    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox2), scroll, TRUE, TRUE, 0);
    gtk_widget_show (scroll);
    
    menu = create_applications_menu ("mate-applications.menu",GTK_CONTAINER(scroll),GTK_BOX(category_box));
    gtk_widget_show_all (window);
    gtk_main();
    return 0;
}
