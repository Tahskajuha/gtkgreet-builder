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

static char *get_string_from_file(GKeyFile *kf, const char *section,
                                  const char *key, gboolean required) {
  GError *error = NULL;
  char *value = g_key_file_get_string(kf, section, key, &error);
  if (value && *value) {
    g_clear_error(&error);
    return value;
  } else if (required) {
    g_error("Missing or invalid '%s' in section [%s]", key, section);
  } else {
    g_clear_error(&error);
    return NULL;
  }
}

static char **get_stringlist_from_file(GKeyFile *kf, const char *section,
                                       const char *key, gsize *size,
                                       gboolean required) {
  GError *error = NULL;
  char **value = g_key_file_get_string_list(kf, section, key, size, &error);
  if (error != NULL && required) {
    g_error("Missing or invalid '%s' in section [%s]", key, section);
  } else if (error != NULL && !required) {
    g_clear_error(&error);
    return NULL;
  } else {
    g_clear_error(&error);
    return value;
  }
}

static gboolean is_auth_widget(char *str) {
  if (g_strcmp0(uimodel->readCommand, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->submit, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->cancel, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->errorPrompt, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->infoPrompt, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->initialAnswer, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(uimodel->pamPromptAnswer, str) == 0) {
    return TRUE;
  }
  return FALSE;
}

static void attach_custom_style(const char *path) {
  GtkCssProvider *provider = gtk_css_provider_new();

  gtk_css_provider_load_from_path(provider, path);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

static void attach_custom_layout(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  GError *error = NULL;

  gtk_builder_add_from_file(builder, path, &error);
  if (error != NULL) {
    g_error("Failed to load layout file: %s", error->message);
  }
  gtkgreet->window->builder = builder;
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  if (!window) {
    g_error("Layout file does not contain required 'main_window' object");
  }
  gtkgreet->window->window = window;

  gtk_window_set_application(GTK_WINDOW(window), gtkgreet->app);
  g_signal_connect(gtkgreet->window->window, "destroy",
                   G_CALLBACK(window_empty), gtkgreet->window);
}

static void read_config() {
  GKeyFile *kf = g_key_file_new();
  GError *error = NULL;
  if (!g_key_file_load_from_file(kf, config, G_KEY_FILE_NONE, &error)) {
    g_error("Failed to load config: %s", error->message);
  }
  g_clear_error(&error);

  // UI file path
  char *layout = get_string_from_file(kf, "ui", "layout", TRUE);
  attach_custom_layout(layout);
  g_free(layout);

  // Style file path
  char *style = get_string_from_file(kf, "ui", "style", TRUE);
  attach_custom_style(style);
  g_free(style);

  // Read auth widgets first
  char *readCommand = get_string_from_file(kf, "auth", "readCommand", TRUE);
  uimodel->readCommand = readCommand;
  char *submit = get_string_from_file(kf, "auth", "submit", TRUE);
  uimodel->submit = submit;
  char *cancel = get_string_from_file(kf, "auth", "cancel", TRUE);
  uimodel->cancel = cancel;
  char *errorPrompt = get_string_from_file(kf, "auth", "errorPrompt", TRUE);
  uimodel->errorPrompt = errorPrompt;
  char *infoPrompt = get_string_from_file(kf, "auth", "infoPrompt", TRUE);
  uimodel->infoPrompt = infoPrompt;
  char *initialAnswer = get_string_from_file(kf, "auth", "initialAnswer", TRUE);
  uimodel->initialAnswer = initialAnswer;
  char *pamPromptAnswer =
      get_string_from_file(kf, "auth", "pamPromptAnswer", TRUE);
  uimodel->pamPromptAnswer = pamPromptAnswer;

  // Read non-essential widget IDs
  char *commandList =
      get_string_from_file(kf, "behaviors", "commandList", FALSE);
  uimodel->commandList = commandList;
  char *poweroff = get_string_from_file(kf, "behaviors", "poweroff", FALSE);
  uimodel->poweroff = poweroff;
  char *suspend = get_string_from_file(kf, "behaviors", "suspend", FALSE);
  uimodel->suspend = suspend;
  char *reboot = get_string_from_file(kf, "behaviors", "reboot", FALSE);
  uimodel->reboot = reboot;

  // Environments list
  gsize env_list_length = 0;
  gchar **commands = get_stringlist_from_file(kf, "session", "environments",
                                              &env_list_length, FALSE);
  config_update_commands_model(commands, env_list_length);
  g_strfreev(commands);

  // List of widgets to be shown in initial state
  gsize initial_state_list_length = 0;
  gchar **initial_state_list = get_stringlist_from_file(
      kf, "state.initial", "visible", &initial_state_list_length, FALSE);
  for (gsize i = 0; i < initial_state_list_length; i++) {
    if (is_auth_widget(initial_state_list[i])) {
      continue;
    }
    g_ptr_array_add(uimodel->initial_state, g_strdup(initial_state_list[i]));
  }
  g_strfreev(initial_state_list);

  // List of widgets to be shown in pam prompt state
  gsize pam_state_list_length = 0;
  gchar **pam_state_list = get_stringlist_from_file(
      kf, "state.pam", "visible", &pam_state_list_length, FALSE);
  for (gsize i = 0; i < pam_state_list_length; i++) {
    if (is_auth_widget(pam_state_list[i])) {
      continue;
    }
    g_ptr_array_add(uimodel->pam_state, g_strdup(pam_state_list[i]));
  }
  g_strfreev(pam_state_list);

  g_key_file_unref(kf);
}

static void activate(GtkApplication *app, gpointer user_data) {
#ifdef LAYER_SHELL
  gtkgreet->use_layer_shell = use_layer_shell;
#endif

  if (!setup_layer_shell()) {
    gtkgreet->window = create_window(NULL);
  }
  if (config != NULL) {
    read_config();
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
