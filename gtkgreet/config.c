#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <stdio.h>

#include "config.h"
#include "gtkgreet.h"

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

void config_update_commands_model(gchar **commands, gsize commandsSize) {
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
