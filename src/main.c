#include <gtk/gtk.h>
#include "menu-window.h"
#include <libintl.h>
#include <locale.h>

#define GETTEXT_PACKAGE "menu-admin"
#define LUNAR_CALENDAR_LOCALEDIR "/usr/share/locale"

int main(int argc,char *argv[])
{
    GtkWidget *window;
    bindtextdomain (GETTEXT_PACKAGE,LUNAR_CALENDAR_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    gtk_init (&argc,&argv);
    window = menu_window_new ();
    g_signal_connect (window,
                     "key-press-event",
                      G_CALLBACK (window_key_press_event_cb),
                      NULL);
    gtk_widget_show (window);
    gtk_main();

    return 0;
}
