#include "app-util.h"

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
