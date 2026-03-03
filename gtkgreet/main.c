#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "config.h"
#include "gtkgreet.h"
#include "uimodel.h"
#include "window.h"

#include <glib/gi18n.h>
#include <locale.h>

struct GtkGreet *gtkgreet = NULL;
struct UiModel *uimodel = NULL;

static char *config = NULL;

#ifdef LAYER_SHELL
static gboolean use_layer_shell = FALSE;
#endif

static GOptionEntry entries[] = {

#ifdef LAYER_SHELL
    {"layer-shell", 'l', 0, G_OPTION_ARG_NONE, &use_layer_shell,
     "Use layer shell", NULL},
#endif
    {"config", 's', 0, G_OPTION_ARG_FILENAME, &config, "Config file to use",
     NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}};

#ifdef LAYER_SHELL
static gboolean setup_layer_shell() {
  if (gtkgreet->use_layer_shell) {
    GdkDisplay *display = gdk_display_get_default();

    // Go through all monitors
    GListModel *monitors = gdk_display_get_monitors(display);
    GdkMonitor *primary = NULL;
    if (g_list_model_get_n_items(monitors) > 0) {
      primary = g_list_model_get_item(monitors, 0);

      gtkgreet->window = create_window(primary);
      g_object_unref(primary);
      g_object_unref(monitors);
    } else {
      g_error("Could not find any monitors");
    }

    return TRUE;
  } else {
    return FALSE;
  }
}
#else
static gboolean setup_layer_shell() { return FALSE; }
#endif

static void activate(GtkApplication *app, gpointer user_data) {
#ifdef LAYER_SHELL
  gtkgreet->use_layer_shell = use_layer_shell;
#endif

  if (!setup_layer_shell()) {
    gtkgreet->window = create_window(NULL);
  }
  if (config != NULL) {
    read_config(config);
  } else {
    g_error("A default fallback will be added soon");
  }

  gtkgreet_activate(gtkgreet);
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  gtkgreet = create_gtkgreet();
  uimodel = create_uimodel();

  g_application_add_main_option_entries(G_APPLICATION(gtkgreet->app), entries);
  g_application_set_option_context_summary(
      G_APPLICATION(gtkgreet->app),
      "GTK-based greeter for greetd. Now with GtkBuilder support!");

  g_signal_connect(gtkgreet->app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(gtkgreet->app), argc, argv);

  gtkgreet_destroy(gtkgreet);

  return status;
}
