#define _POSIX_C_SOURCE 200809L

#include <assert.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "gtkgreet.h"
#include "uimodel.h"
#include "window.h"

#ifdef LAYER_SHELL

#include <gtk4-layer-shell.h>

void window_setup_layershell(struct Window *ctx) {
  GtkEventController *motion = gtk_event_controller_motion_new();
  gtk_widget_add_controller(ctx->window, motion);

  gtk_layer_init_for_window(GTK_WINDOW(ctx->window));
  gtk_layer_set_layer(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_monitor(GTK_WINDOW(ctx->window), ctx->monitor);
  gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(ctx->window));
  gtk_layer_set_anchor(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_LEFT,
                       TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_RIGHT,
                       TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_BOTTOM,
                       TRUE);
}

#endif

static void hide_all_widgets(struct Window *ctx) {
  gtk_widget_set_visible(uimodel->widgets[INITIAL_ANSWER].w, FALSE);
  gtk_widget_set_visible(uimodel->widgets[READ_COMMAND].w, FALSE);
  gtk_widget_set_visible(uimodel->widgets[PAM_PROMPT_ANSWER].w, FALSE);
  for (guint i = 0; i < uimodel->initial_state->len; i++) {
    char *id = g_ptr_array_index(uimodel->initial_state, i);
    GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
    if (w) {
      gtk_widget_set_visible(w, FALSE);
    }
  }
  for (guint i = 0; i < uimodel->pam_state->len; i++) {
    char *id = g_ptr_array_index(uimodel->pam_state, i);
    GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
    if (w) {
      gtk_widget_set_visible(w, FALSE);
    }
  }
}

static void update_text(GtkWidget *w, const char *text) {
  if (GTK_IS_LABEL(w)) {
    gtk_label_set_text(GTK_LABEL(w), text);
  } else if (GTK_IS_EDITABLE(w)) {
    gtk_editable_set_text(GTK_EDITABLE(w), text);
  } else if (GTK_IS_TEXT_VIEW(w)) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", 1);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(w), &end, 0.0, FALSE, 0, 0);
  }
}

static GtkWidget *find_first_editable(GtkWidget *widget) {
  if (!gtk_widget_get_visible(widget))
    return NULL;
  if (GTK_IS_EDITABLE(widget))
    return widget;

  for (GtkWidget *child = gtk_widget_get_first_child(widget); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *result = find_first_editable(child);
    if (result)
      return result;
  }

  return NULL;
}

void window_setup_question(struct Window *ctx, enum QuestionType type,
                           char *question, char *error, char *info) {
  hide_all_widgets(ctx);

  switch (type) {
  case QuestionTypeInitial: {
    gtk_widget_set_visible(uimodel->widgets[READ_COMMAND].w, TRUE);
    gtk_widget_set_visible(uimodel->widgets[INITIAL_ANSWER].w, TRUE);
    for (guint i = 0; i < uimodel->initial_state->len; i++) {
      char *id = g_ptr_array_index(uimodel->initial_state, i);
      GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
      if (w) {
        gtk_widget_set_visible(w, TRUE);
      }
    }
    break;
  }
  case QuestionTypePamPrompt: {
    gtk_widget_set_visible(uimodel->widgets[PAM_PROMPT_ANSWER].w, TRUE);
    for (guint i = 0; i < uimodel->pam_state->len; i++) {
      char *id = g_ptr_array_index(uimodel->pam_state, i);
      GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
      if (w) {
        gtk_widget_set_visible(w, TRUE);
      }
    }
    break;
  }
  }

  if (info && uimodel->widgets[INFO_PROMPT].w) {
    update_text(uimodel->widgets[INFO_PROMPT].w, info);
    gtk_widget_set_visible(uimodel->widgets[INFO_PROMPT].w, TRUE);
  }
  if (error && uimodel->widgets[ERROR_PROMPT].w) {
    update_text(uimodel->widgets[ERROR_PROMPT].w, error);
    gtk_widget_set_visible(uimodel->widgets[ERROR_PROMPT].w, TRUE);
  }
  if (question && uimodel->widgets[QUESTION_PROMPT].w) {
    update_text(uimodel->widgets[QUESTION_PROMPT].w, question);
    gtk_widget_set_visible(uimodel->widgets[QUESTION_PROMPT].w, TRUE);
  }

  gtk_window_present(GTK_WINDOW(ctx->window));

  GtkWidget *editable = find_first_editable(ctx->window);
  if (editable) {
    gtk_widget_grab_focus(editable);
  }
}

void window_empty(struct Window *ctx) {
  // I don't know why I am still keeping this; might be useful if window
  // teardown logic is needed later :P
}

struct Window *create_window(GdkMonitor *monitor) {
  struct Window *w = calloc(1, sizeof(struct Window));
  if (w == NULL) {
    fprintf(stderr, "failed to allocate Window instance\n");
    exit(1);
  }
  w->monitor = monitor;

  return w;
}
