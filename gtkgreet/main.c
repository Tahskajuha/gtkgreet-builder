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
    } else {
      g_error("Could not find any monitors");
    }

    window_configure(gtkgreet->window);
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

static void attach_custom_layout(const char *path) {}

static void read_config() {
  GKeyFile *kf = g_key_file_new();

  GError *error = NULL;
  if (!g_key_file_load_from_file(kf, config, G_KEY_FILE_NONE, &error)) {
    g_error("Failed to load config: %s", error->message);
  }
  g_clear_error(&error);

  // Environments list
  gsize env_list_length = 0;
  gchar **commands = g_key_file_get_string_list(kf, "session", "environments",
                                                &env_list_length, &error);
  if (error != NULL) {
    commands = NULL;
  }
  config_update_commands_model(commands, env_list_length);
  g_strfreev(commands);
  g_clear_error(&error);

  // Style file path
  char *style = g_key_file_get_string(kf, "ui", "style", &error);
  if (style && *style) {
    attach_custom_style(style);
  }
  g_free(style);
  g_clear_error(&error);

  // UI file path
  char *layout = g_key_file_get_string(kf, "ui", "layout", &error);
  if (layout && *layout) {
    attach_custom_layout(layout);
  }
  g_free(layout);
  g_clear_error(&error);

  // ID names
  char *readCommand = g_key_file_get_string(kf, "roles", "readCommand", &error);
  if (readCommand && *readCommand) {
    uimodel->readCommand = readCommand;
  } else {
    uimodel->readCommand = NULL;
    g_free(readCommand);
  }
  g_clear_error(&error);

  char *commandList = g_key_file_get_string(kf, "roles", "commandList", &error);
  if (commandList && *commandList) {
    uimodel->commandList = commandList;
  } else {
    uimodel->commandList = NULL;
    g_free(commandList);
  }
  g_clear_error(&error);

  g_key_file_unref(kf);
}

static void activate(GtkApplication *app, gpointer user_data) {
#ifdef LAYER_SHELL
  gtkgreet->use_layer_shell = use_layer_shell;
#endif

  if (config != NULL) {
    read_config();
  } else {
    g_error("A default configuration will be added soon");
  }

  gtkgreet_activate(gtkgreet);
  if (!setup_layer_shell()) {
    gtkgreet->window = create_window(NULL);
    window_configure(gtkgreet->window);
  }
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
