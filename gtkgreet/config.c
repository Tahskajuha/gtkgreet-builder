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

static gboolean is_valid_root(GtkWidget *w) {
  return GTK_IS_BOX(w) || GTK_IS_GRID(w) || GTK_IS_OVERLAY(w) ||
         GTK_IS_STACK(w) || GTK_IS_FIXED(w) || GTK_IS_CENTER_BOX(w) ||
         GTK_IS_PANED(w);
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
  uimodel->widgets[ROOT].w = root;
  uimodel->widgets[INITIAL_ANSWER].w = initialAnswer;
  uimodel->widgets[PAM_PROMPT_ANSWER].w = pamPromptAnswer;
  uimodel->widgets[READ_COMMAND].w = readCommand;

  bind_actions(gtkgreet->window);
}

static int populate_uimodel(GKeyFile *kf) {
  GtkWidget *fb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 40);
  g_object_ref_sink(fb);
  gboolean fbUsed = false;
  for (enum role r = ROOT; r < ROLE_COUNT; r++) {
    GtkWidget *w = NULL;
    char *id = get_string_from_file(kf, uimodel->widgets[r].section,
                                    uimodel->widgets[r].field);
    if (id) {
      w = GTK_WIDGET(gtk_builder_get_object(gtkgreet->window->builder, id));
      g_free(id);
    }
    switch (r) {
    case ROOT:
      if (!w || !is_valid_root(w)) {
        critical_fallback();
        g_object_unref(fb);
        return -1;
      }
      gtk_window_set_child(GTK_WINDOW(gtkgreet->window->window), w);
      break;
    case INITIAL_ANSWER:
    case PAM_PROMPT_ANSWER:
    case READ_COMMAND:
      if (!w || !is_readable_widget(w)) {
        if (w)
          gtk_widget_set_visible(w, FALSE);
        w = r == INITIAL_ANSWER
                ? create_fbInitialAnswer()
                : (r == PAM_PROMPT_ANSWER ? create_fbPamPromptAnswer()
                                          : create_fbReadCommand());
        gtk_box_append(GTK_BOX(fb), w);
        fbUsed = true;
      }
      break;
    default:
      break;
    }
    uimodel->widgets[r].w = w;
  }
  if (fbUsed) {
    push_widget_into_root(GTK_BOX(fb));
  } else {
    g_object_unref(fb);
  }
  return 0;
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

  GtkWidget *window = gtk_application_window_new(gtkgreet->app);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtkgreet->window->builder = builder;
  gtkgreet->window->window = window;
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

  // UI file path
  char *layout = get_string_from_file(kf, "ui", "layout");
  if (attach_custom_layout(layout) != 0) {
    g_free(layout);

    g_key_file_unref(kf);
    return;
  }
  g_free(layout);

  // Style file path
  char *style = get_string_from_file(kf, "ui", "style");
  attach_custom_style(style);
  g_free(style);

  if (populate_uimodel(kf) != 0) {
    g_key_file_unref(kf);
    return;
  }

  // Environments list
  gsize env_list_length = 0;
  gchar **commands =
      get_stringlist_from_file(kf, "session", "environments", &env_list_length);
  config_update_commands_model(commands, env_list_length);
  g_strfreev(commands);

  bind_actions(gtkgreet->window);

  struct CriticalRoleIds critIDs = (struct CriticalRoleIds){
      .root = get_string_from_file(kf, "core", "root"),
      .readCommand = get_string_from_file(kf, "core", "read_command"),
      .initialAnswer = get_string_from_file(kf, "core", "initial_answer"),
      .pamPromptAnswer = get_string_from_file(kf, "core", "pam_prompt_answer")};

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

  g_free(critIDs.root);
  g_free(critIDs.readCommand);
  g_free(critIDs.initialAnswer);
  g_free(critIDs.pamPromptAnswer);

  g_key_file_unref(kf);
}
