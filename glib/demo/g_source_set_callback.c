#include <glib.h>

gboolean callback(gpointer userdata)
{
    int *pint = (int *)userdata;
    g_print("callback called: %d\n", (*pint)++);
    
    if (*pint < 5) {
        return TRUE;
    } else {
        g_print("reach 5\tstop!\n");
        return FALSE;
    }
}

int main(int argc, char *argv[])
{
    GMainLoop *loop;
    GSource *source;
    gint count = 0;
    guint id;

    loop = g_main_loop_new(NULL, FALSE);

    source = g_timeout_source_new(1000);    
    g_print("%d\n", count++);

    id = g_source_attach(source, NULL);
    g_source_set_callback(source, callback, &count, NULL);

    g_print("id: %u\n", id);

    g_main_loop_run(loop);
    g_print("loop over\n");
    g_main_loop_unref(loop);

    return 0;
}

