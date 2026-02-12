#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "config.h"
#include "gtkgreet.h"
#include "window.h"

#include <glib/gi18n.h>
#include <locale.h>

struct GtkGreet *gtkgreet = NULL;

static char *config = NULL;

#ifdef LAYER_SHELL
static gboolean use_layer_shell = FALSE;
#endif

// Array of GOptionEntries (glib's command-line argument parser) specifying
// whether to use layer shell, the command, background image and stylesheet

static GOptionEntry entries[] = {

#ifdef LAYER_SHELL
    {"layer-shell", 'l', 0, G_OPTION_ARG_NONE, &use_layer_shell,
     "Use layer shell", NULL},
#endif
    {"config", 's', 0, G_OPTION_ARG_FILENAME, &config, "Config file to use",
     NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}};

#ifdef LAYER_SHELL
static void reload_outputs() {
  GdkDisplay *display = gdk_display_get_default();

  // Make note of all existing windows
  GArray *dead_windows = g_array_new(FALSE, TRUE, sizeof(struct Window *));
  for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
    struct Window *ctx = g_array_index(gtkgreet->windows, struct Window *, idx);
    g_array_append_val(dead_windows, ctx);
  }

  // Go through all monitors
  for (int i = 0; i < gdk_display_get_n_monitors(display); i++) {
    GdkMonitor *monitor = gdk_display_get_monitor(display, i);
    struct Window *w = gtkgreet_window_by_monitor(gtkgreet, monitor);
    if (w != NULL) {
      // We already have this monitor, remove from dead_windows list
      for (guint ydx = 0; ydx < dead_windows->len; ydx++) {
        if (w == g_array_index(dead_windows, struct Window *, ydx)) {
          g_array_remove_index_fast(dead_windows, ydx);
          break;
        }
      }
    } else {
      create_window(monitor);
    }
  }

  // Remove all windows left behind
  for (guint idx = 0; idx < dead_windows->len; idx++) {
    struct Window *w = g_array_index(dead_windows, struct Window *, idx);
    gtk_widget_destroy(w->window);
    if (gtkgreet->focused_window == w) {
      gtkgreet->focused_window = NULL;
    }
  }

  for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
    struct Window *win = g_array_index(gtkgreet->windows, struct Window *, idx);
    window_configure(win);
  }

  g_array_unref(dead_windows);
}

static void monitors_changed(GdkDisplay *display, GdkMonitor *monitor) {
  reload_outputs();
}

static gboolean setup_layer_shell() {
  if (gtkgreet->use_layer_shell) {
    reload_outputs();
    GdkDisplay *display = gdk_display_get_default();
    g_signal_connect(display, "monitor-added", G_CALLBACK(monitors_changed),
                     NULL);
    g_signal_connect(display, "monitor-removed", G_CALLBACK(monitors_changed),
                     NULL);
    return TRUE;
  } else {
    return FALSE;
  }
}
#else
static gboolean setup_layer_shell() { return FALSE; }
#endif

static void attach_custom_style(const char *path) {
  GtkCssProvider *provider = gtk_css_provider_new();

  gtk_css_provider_load_from_path(provider, path);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

static void activate(GtkApplication *app, gpointer user_data) {
#ifdef LAYER_SHELL
  gtkgreet->use_layer_shell = use_layer_shell;
#endif

  if (config != NULL) {
    GKeyFile *kf = g_key_file_new();

    GError *error = NULL;
    if (!g_key_file_load_from_file(kf, config, G_KEY_FILE_NONE, &error)) {
      g_error("Failed to load config: %s", error->message);
    }

    error = NULL;
    gsize env_list_length = 0;
    gchar **commands = g_key_file_get_string_list(kf, "session", "environments",
                                                  &env_list_length, &error);
    if (error != NULL) {
      g_clear_error(&error);
      commands = NULL;
    }
    config_update_commands_model(commands, env_list_length);
    g_strfreev(commands);

    error = NULL;
    char *style = g_key_file_get_string(kf, "ui.style", "css", &error);
    if (style != NULL && error == NULL) {
      attach_custom_style(style);
      g_free(style);
    } else {
      g_clear_error(&error);
    }

    g_key_file_unref(kf);
  } else {
    g_error("A default configuration will be added soon");
  }

  gtkgreet_activate(gtkgreet);
  if (!setup_layer_shell()) {
    struct Window *win = create_window(NULL);
    gtkgreet_focus_window(gtkgreet, win);
    window_configure(win);
  }
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  gtkgreet = create_gtkgreet();

  g_application_add_main_option_entries(G_APPLICATION(gtkgreet->app), entries);
  g_application_set_option_context_summary(
      G_APPLICATION(gtkgreet->app),
      "GTK-based greeter for greetd. Now with GtkBuilder support!");

  g_signal_connect(gtkgreet->app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(gtkgreet->app), argc, argv);

  gtkgreet_destroy(gtkgreet);

  return status;
}
