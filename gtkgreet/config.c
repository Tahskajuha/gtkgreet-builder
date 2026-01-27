#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <stdio.h>

#include "config.h"
#include "gtkgreet.h"

int config_update_command_selector(GtkStringList *commandList) {
  int entries = 0;
  if (gtkgreet->command != NULL) {
    gtk_string_list_append(commandList, gtkgreet->command);
    entries++;
  }

  char buffer[255];
  FILE *fp = fopen("/etc/greetd/environments", "r");
  if (fp == NULL) {
    return entries;
  }
  while (fgets(buffer, 255, (FILE *)fp)) {
    size_t len = strnlen(buffer, 255);
    if (len > 0 && len < 255 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    if (gtkgreet->command != NULL && strcmp(gtkgreet->command, buffer) == 0) {
      continue;
    }
    gtk_string_list_append(commandList, buffer);
    entries++;
  }

  fclose(fp);
  return entries;
}
