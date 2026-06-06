#define _POSIX_C_SOURCE 200809L

#include "fbWidgets.h"
#include <gtk/gtk.h>

GtkWidget *create_fbInitialAnswer() {
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Username");
  return entry;
}

GtkWidget *create_fbPamPromptAnswer() {
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Password");
  gtk_entry_set_visibility(GTK_ENTRY(entry), false);
  return entry;
}

GtkWidget *create_fbReadCommand() {
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Command");
  return entry;
}

GtkWidget *create_fbRoot() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 40);
  return box;
}
