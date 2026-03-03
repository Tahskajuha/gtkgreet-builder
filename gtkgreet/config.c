#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <stdio.h>

#include "config.h"
#include "gtkgreet.h"
#include "uimodel.h"
#include "window.h"

static gboolean is_duplicate(const char *str) {
  guint length = g_list_model_get_n_items(G_LIST_MODEL(gtkgreet->commandList));
  for (guint i = 0; i < length; i++) {
    if (g_strcmp0(str, gtk_string_list_get_string(gtkgreet->commandList, i)) ==
        0) {
      return TRUE;
    }
  }
  return FALSE;
}

static void widget_exists(GtkBuilder *builder, const char *str) {
  if (!gtk_builder_get_object(builder, str))
    g_error("Layout file does not contain required %s object", str);
}

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

static gboolean is_auth_widget(const char *str) {
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
  widget_exists(builder, "main_window");
  widget_exists(builder, uimodel->readCommand);
  widget_exists(builder, uimodel->submit);
  widget_exists(builder, uimodel->cancel);
  widget_exists(builder, uimodel->errorPrompt);
  widget_exists(builder, uimodel->infoPrompt);
  widget_exists(builder, uimodel->initialAnswer);
  widget_exists(builder, uimodel->pamPromptAnswer);
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  gtkgreet->window->window = window;

  gtk_window_set_application(GTK_WINDOW(window), gtkgreet->app);
  g_signal_connect(gtkgreet->window->window, "destroy",
                   G_CALLBACK(window_empty), gtkgreet->window);
}

static void config_update_commands_model(gchar **commands, gsize commandsSize) {
  guint n = g_list_model_get_n_items(G_LIST_MODEL(gtkgreet->commandList));
  if (n > 0) {
    gtk_string_list_splice(gtkgreet->commandList, 0, n, NULL);
  }
  char buffer[255];
  for (gsize i = 0; i < commandsSize; i++) {
    if (!is_duplicate(commands[i])) {
      gtk_string_list_append(gtkgreet->commandList, commands[i]);
    }
  }
  FILE *fp = fopen("/etc/greetd/environments", "r");
  if (fp == NULL) {
    return;
  }
  while (fgets(buffer, 255, (FILE *)fp)) {
    size_t len = strnlen(buffer, 255);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    if (buffer[0] == '\0' || buffer[0] == '#') {
      continue;
    }
    if (!is_duplicate(buffer)) {
      gtk_string_list_append(gtkgreet->commandList, buffer);
    }
  }

  fclose(fp);
}

void read_config(const char *config) {
  GKeyFile *kf = g_key_file_new();
  GError *error = NULL;
  if (!g_key_file_load_from_file(kf, config, G_KEY_FILE_NONE, &error)) {
    g_error("Failed to load config: %s", error->message);
  }
  g_clear_error(&error);

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

  // UI file path
  char *layout = get_string_from_file(kf, "ui", "layout", TRUE);
  attach_custom_layout(layout);
  g_free(layout);

  // Style file path
  char *style = get_string_from_file(kf, "ui", "style", TRUE);
  attach_custom_style(style);
  g_free(style);

  g_key_file_unref(kf);
}
