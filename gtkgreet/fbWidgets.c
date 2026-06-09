#define _POSIX_C_SOURCE 200809L

#include "fbWidgets.h"
#include "uimodel.h"
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

static void collect_essential_widgets_in_subtree(GtkWidget *w, GPtrArray *out) {
  if (!w)
    return;
  if (w == uimodel->widgets[INITIAL_ANSWER].w ||
      w == uimodel->widgets[PAM_PROMPT_ANSWER].w ||
      w == uimodel->widgets[READ_COMMAND].w) {
    g_ptr_array_add(out, w);
  }
  for (GtkWidget *i = gtk_widget_get_first_child(w); i;
       i = gtk_widget_get_next_sibling(i)) {
    collect_essential_widgets_in_subtree(i, out);
  }
}

void push_widget_into_root(GtkBox *w) {
  if (GTK_IS_BOX(uimodel->widgets[ROOT].w)) {
    gtk_box_prepend(GTK_BOX(uimodel->widgets[ROOT].w), GTK_WIDGET(w));
  } else if (GTK_IS_GRID(uimodel->widgets[ROOT].w)) {
    int max_row = -1;
    for (GtkWidget *i = gtk_widget_get_first_child(uimodel->widgets[ROOT].w); i;
         i = gtk_widget_get_next_sibling(i)) {
      int row;
      g_object_get(i, "top-attach", &row, NULL);
      if (row > max_row)
        max_row = row;
    }
    gtk_grid_attach(GTK_GRID(uimodel->widgets[ROOT].w), GTK_WIDGET(w), 0, max_row + 1, 1,
                    1);
  } else if (GTK_IS_OVERLAY(uimodel->widgets[ROOT].w)) {
    gtk_overlay_add_overlay(GTK_OVERLAY(uimodel->widgets[ROOT].w), GTK_WIDGET(w));
  } else if (GTK_IS_STACK(uimodel->widgets[ROOT].w)) {
    gtk_stack_add_child(GTK_STACK(uimodel->widgets[ROOT].w), GTK_WIDGET(w));
  } else if (GTK_IS_FIXED(uimodel->widgets[ROOT].w)) {
    gtk_fixed_put(GTK_FIXED(uimodel->widgets[ROOT].w), GTK_WIDGET(w), 0, 0);
  } else if (GTK_IS_CENTER_BOX(uimodel->widgets[ROOT].w)) {
    GPtrArray *essentials = g_ptr_array_new();
    collect_essential_widgets_in_subtree(
        gtk_center_box_get_start_widget(GTK_CENTER_BOX(uimodel->widgets[ROOT].w)),
        essentials);
    for (guint i = 0; i < essentials->len; i++) {
      GtkWidget *essentialWidget = g_ptr_array_index(essentials, i);
      gtk_box_append(w, essentialWidget);
    }
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(uimodel->widgets[ROOT].w),
                                    GTK_WIDGET(w));
    g_ptr_array_free(essentials, FALSE);
  } else if (GTK_IS_PANED(uimodel->widgets[ROOT].w)) {
    GPtrArray *essentials = g_ptr_array_new();
    collect_essential_widgets_in_subtree(
        gtk_paned_get_start_child(GTK_PANED(uimodel->widgets[ROOT].w)), essentials);
    for (guint i = 0; i < essentials->len; i++) {
      GtkWidget *essentialWidget = g_ptr_array_index(essentials, i);
      gtk_box_append(w, essentialWidget);
    }
    gtk_paned_set_start_child(GTK_PANED(uimodel->widgets[ROOT].w), GTK_WIDGET(w));
    g_ptr_array_free(essentials, FALSE);
  }
}
