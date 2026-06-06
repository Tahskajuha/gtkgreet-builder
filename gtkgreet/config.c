#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <stdio.h>

#include "config.h"
#include "fbWidgets.h"
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

static gboolean is_readable_widget(GtkWidget *w) {
  return GTK_IS_EDITABLE(w) || GTK_IS_DROP_DOWN(w) || GTK_IS_LIST_VIEW(w);
}

static GtkWidget *get_widget(const char *id, enum role role) {
  GtkWidget *w = NULL;
  if (id) {
    w = GTK_WIDGET(gtk_builder_get_object(gtkgreet->window->builder, id));
  }

  switch (role) {
  case INITIAL_ANSWER:
    if (!w || !is_readable_widget(w)) {
      w = create_fbInitialAnswer();
    }
    break;
  case PAM_PROMPT_ANSWER:
    if (!w || !is_readable_widget(w)) {
      w = create_fbPamPromptAnswer();
    }
    break;
  case READ_COMMAND:
    if (!w || !is_readable_widget(w)) {
      w = create_fbReadCommand();
    }
    break;
  case COMMAND_LIST:
  case POWEROFF:
  case SUSPEND:
  case REBOOT:
  case HIBERNATE:
  case CANCEL:
  case ERROR_PROMPT:
  case INFO_PROMPT:
  case QUESTION_PROMPT:
  case SUBMIT:
    break;
  default:
    g_error("Invalid role in get_widget");
  }
  return w;
}

static char *get_string_from_file(GKeyFile *kf, const char *section,
                                  const char *key) {
  GError *error = NULL;
  char *value = g_key_file_get_string(kf, section, key, &error);
  if (value && *value) {
    g_clear_error(&error);
    return value;
  } else {
    g_free(value);
    g_clear_error(&error);
    return NULL;
  }
}

static char **get_stringlist_from_file(GKeyFile *kf, const char *section,
                                       const char *key, gsize *size) {
  GError *error = NULL;
  char **value = g_key_file_get_string_list(kf, section, key, size, &error);
  if (error != NULL) {
    g_clear_error(&error);
    return NULL;
  } else {
    g_clear_error(&error);
    return value;
  }
}

static gboolean is_auth_widget(struct CriticalRoleIds c, const char *str) {
  if (g_strcmp0(c.readCommand, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(c.initialAnswer, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(c.pamPromptAnswer, str) == 0) {
    return TRUE;
  } else if (g_strcmp0(c.root, str) == 0) {
    return TRUE;
  }
  return FALSE;
}

static void attach_custom_style(const char *path) {
  if (!path)
    return;
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, path);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
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

static void critical_fallback() {
  GtkWidget *window = gtk_application_window_new(gtkgreet->app);
  GtkWidget *root = create_fbRoot();
  GtkWidget *initialAnswer = create_fbInitialAnswer();
  GtkWidget *pamPromptAnswer = create_fbPamPromptAnswer();
  GtkWidget *readCommand = create_fbReadCommand();

  gtk_window_set_child(GTK_WINDOW(window), root);
  gtk_box_append(GTK_BOX(root), initialAnswer);
  gtk_box_append(GTK_BOX(root), pamPromptAnswer);
  gtk_box_append(GTK_BOX(root), readCommand);

  gtkgreet->window->window = window;
  uimodel->root = root;
  uimodel->initialAnswer = initialAnswer;
  uimodel->pamPromptAnswer = pamPromptAnswer;
  uimodel->readCommand = readCommand;

  bind_actions(gtkgreet->window);
}

static int attach_custom_layout(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  GError *error = NULL;

  if (!path || !gtk_builder_add_from_file(builder, path, &error)) {
    g_warning("Failed to load layout file: %s",
              error ? error->message : "unknown error");
    g_object_unref(builder);
    g_clear_error(&error);
    critical_fallback();
    return -1;
  }

  GtkWidget *root = GTK_WIDGET(gtk_builder_get_object(builder, "main_root"));
  if (!root || GTK_IS_WINDOW(root) || GTK_IS_POPOVER(root)) {
    g_warning("Invalid or missing main_root object in layout file");
    g_object_unref(builder);
    critical_fallback();
    return -1;
  }

  GtkWidget *window = gtk_application_window_new(gtkgreet->app);
  gtkgreet->window->builder = builder;
  gtkgreet->window->window = window;
  gtk_window_set_child(GTK_WINDOW(window), root);
  uimodel->root = root;
  bind_actions(gtkgreet->window);
  g_clear_error(&error);
  return 0;
}

void resolve_config(const char *config) {
  if (!config) {
    g_warning("No config specified. Using fallback setup");
    critical_fallback();
    return;
  }

  GKeyFile *kf = g_key_file_new();
  GError *error = NULL;
  if (!g_key_file_load_from_file(kf, config, G_KEY_FILE_NONE, &error)) {
    g_warning("Failed to load config: %s", error->message);
    critical_fallback();
    g_key_file_unref(kf);
    g_clear_error(&error);
    return;
  }
  g_clear_error(&error);

  struct CriticalRoleIds critIDs = (struct CriticalRoleIds){
      .root = "main_root",
      .readCommand = get_string_from_file(kf, "core", "read_command"),
      .initialAnswer = get_string_from_file(kf, "core", "initial_answer"),
      .pamPromptAnswer = get_string_from_file(kf, "core", "pam_prompt_answer")};

  // UI file path
  char *layout = get_string_from_file(kf, "ui", "layout");
  if (attach_custom_layout(layout) != 0) {
    g_free(layout);

    g_free(critIDs.readCommand);
    g_free(critIDs.initialAnswer);
    g_free(critIDs.pamPromptAnswer);

    g_key_file_unref(kf);
    return;
  }
  g_free(layout);

  // Style file path
  char *style = get_string_from_file(kf, "ui", "style");
  attach_custom_style(style);
  g_free(style);

  // Read auth widgets first
  uimodel->initialAnswer = get_widget(critIDs.initialAnswer, INITIAL_ANSWER);
  uimodel->pamPromptAnswer =
      get_widget(critIDs.pamPromptAnswer, PAM_PROMPT_ANSWER);
  uimodel->readCommand = get_widget(critIDs.readCommand, READ_COMMAND);

  // Read non-essential widget IDs
  char *commandListID = get_string_from_file(kf, "optional", "command_list");
  uimodel->commandList = get_widget(commandListID, COMMAND_LIST);
  g_free(commandListID);
  char *powerOffID = get_string_from_file(kf, "optional", "poweroff");
  uimodel->poweroff = get_widget(powerOffID, POWEROFF);
  g_free(powerOffID);
  char *suspendID = get_string_from_file(kf, "optional", "suspend");
  uimodel->suspend = get_widget(suspendID, SUSPEND);
  g_free(suspendID);
  char *rebootID = get_string_from_file(kf, "optional", "reboot");
  uimodel->reboot = get_widget(rebootID, REBOOT);
  g_free(rebootID);
  char *hibernateID = get_string_from_file(kf, "optional", "hibernate");
  uimodel->hibernate = get_widget(hibernateID, HIBERNATE);
  g_free(hibernateID);
  char *cancelID = get_string_from_file(kf, "optional", "cancel");
  uimodel->cancel = get_widget(cancelID, CANCEL);
  g_free(cancelID);
  char *errorPromptID = get_string_from_file(kf, "optional", "error_prompt");
  uimodel->errorPrompt = get_widget(errorPromptID, ERROR_PROMPT);
  g_free(errorPromptID);
  char *infoPromptID = get_string_from_file(kf, "optional", "info_prompt");
  uimodel->infoPrompt = get_widget(infoPromptID, INFO_PROMPT);
  g_free(infoPromptID);
  char *questionPromptID =
      get_string_from_file(kf, "optional", "question_prompt");
  uimodel->questionPrompt = get_widget(questionPromptID, QUESTION_PROMPT);
  g_free(questionPromptID);
  char *submitID = get_string_from_file(kf, "optional", "submit");
  uimodel->submit = get_widget(submitID, SUBMIT);
  g_free(submitID);

  // Environments list
  gsize env_list_length = 0;
  gchar **commands =
      get_stringlist_from_file(kf, "session", "environments", &env_list_length);
  config_update_commands_model(commands, env_list_length);
  g_strfreev(commands);

  // List of widgets to be shown in initial state
  gsize initial_state_list_length = 0;
  gchar **initial_state_list = get_stringlist_from_file(
      kf, "state.initial", "visible", &initial_state_list_length);
  for (gsize i = 0; i < initial_state_list_length; i++) {
    if (is_auth_widget(critIDs, initial_state_list[i])) {
      continue;
    }
    g_ptr_array_add(uimodel->initial_state, g_strdup(initial_state_list[i]));
  }
  g_strfreev(initial_state_list);

  // List of widgets to be shown in pam prompt state
  gsize pam_state_list_length = 0;
  gchar **pam_state_list = get_stringlist_from_file(kf, "state.pam", "visible",
                                                    &pam_state_list_length);
  for (gsize i = 0; i < pam_state_list_length; i++) {
    if (is_auth_widget(critIDs, pam_state_list[i])) {
      continue;
    }
    g_ptr_array_add(uimodel->pam_state, g_strdup(pam_state_list[i]));
  }
  g_strfreev(pam_state_list);

  g_free(critIDs.readCommand);
  g_free(critIDs.initialAnswer);
  g_free(critIDs.pamPromptAnswer);

  g_key_file_unref(kf);
}
