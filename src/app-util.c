#include "app-util.h"

#define MENU_GLIB_STR_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

static char *app_xdg_icon_remove_extension (const char *icon)
{
    char *icon_no_extension;
    char *p;

    icon_no_extension = g_strdup (icon);
    p = strrchr (icon_no_extension, '.');
    if (p &&
        (strcmp (p, ".png") == 0 ||
         strcmp (p, ".xpm") == 0 ||
         strcmp (p, ".svg") == 0)) {
        *p = 0;
    }

    return icon_no_extension;
}

GIcon *app_gicon_from_icon_name (const char *icon_name) 
{
    GIcon *icon = NULL;
    if (icon_name == NULL) 
    {
        return NULL;
    }

    if (g_path_is_absolute(icon_name)) 
    {
        if (!g_file_test (icon_name, G_FILE_TEST_EXISTS)) 
        {
            gchar *name = g_path_get_basename (icon_name);
            icon = g_themed_icon_new (name);
            g_free (name);
        }
        else 
        {
            GFile *gfile = g_file_new_for_path (icon_name);
            icon = g_file_icon_new (gfile);
            g_object_unref (gfile);
        }
    }
    else 
    {
        gchar *name = app_xdg_icon_remove_extension (icon_name);
        icon = g_themed_icon_new (name);
        g_free (name);
    }
    return icon;
}
static gboolean
menu_util_query_tooltip_cb (GtkWidget  *widget,
                            gint        x,
                            gint        y,
                            gboolean    keyboard_tip,
                            GtkTooltip *tooltip,
                            const char *text)
{
    gtk_tooltip_set_text (tooltip, text);
    return TRUE;
}

void menu_util_set_tooltip_text (GtkWidget  *widget,
                                 const char *text)
{
    g_signal_handlers_disconnect_matched (widget,
                                          G_SIGNAL_MATCH_FUNC,
                                          0, 0, NULL,
                                          menu_util_query_tooltip_cb,
                                          NULL);

    if (g_utf8_strlen (text,-1) <= 14)
        return;
    if (MENU_GLIB_STR_EMPTY (text)) {
        g_object_set (widget, "has-tooltip", FALSE, NULL);
        return;
    }

    g_object_set (widget, "has-tooltip", TRUE, NULL);
    g_signal_connect_data (widget,
                          "query-tooltip",
                           G_CALLBACK (menu_util_query_tooltip_cb),
                           g_strdup (text),
                          (GClosureNotify) G_CALLBACK (g_free),
                           0);
}
GtkWidget *set_button_style(const gchar *button_text,const gchar *icon_name)
{
    GtkWidget *button;
    GtkWidget *image;

    button = gtk_button_new ();
    image  = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (button), image);

    return button;
}
