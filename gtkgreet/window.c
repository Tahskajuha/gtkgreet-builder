#define _POSIX_C_SOURCE 200809L

#include <assert.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "gtkgreet.h"
#include "uimodel.h"
#include "window.h"

#include <gtk4-layer-shell.h>

#ifdef LAYER_SHELL

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
  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->readCommand)),
      FALSE);
  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->submit)), FALSE);
  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->initialAnswer)),
      FALSE);
  gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(
                             ctx->builder, uimodel->pamPromptAnswer)),
                         FALSE);
  for (guint i = 0; i < uimodel->initial_state->len; i++) {
    char *id = g_ptr_array_index(uimodel->initial_state, i);
    GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
    if (w) {
      gtk_widget_set_visible(w, FALSE);
    }
  }
  for (guint i = 0; i < uimodel->pam_state->len; i++) {
    char *id = g_ptr_array_index(uimodel->pam_state, i);
    g_print("Hiding %s", id);
    GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ctx->builder, id));
    if (w) {
      gtk_widget_set_visible(w, FALSE);
    }
  }
}

static void set_text(GtkWidget *w, const char *text) {
  if (GTK_IS_LABEL(w)) {
    gtk_label_set_text(GTK_LABEL(w), text);
  } else if (GTK_IS_EDITABLE(w)) {
    gtk_editable_set_text(GTK_EDITABLE(w), text);
  } else if (GTK_IS_TEXT_VIEW(w)) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
    gtk_text_buffer_set_text(buffer, text, -1);
  }
}

void window_setup_question(struct Window *ctx, enum QuestionType type,
                           char *question, char *error, char *info) {
  hide_all_widgets(ctx);

  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->submit)), TRUE);
  switch (type) {
  case QuestionTypeInitial: {
    gtk_widget_set_visible(
        GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->readCommand)),
        TRUE);
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(
                               ctx->builder, uimodel->initialAnswer)),
                           TRUE);
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
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(
                               ctx->builder, uimodel->pamPromptAnswer)),
                           TRUE);
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
  GtkWidget *q =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->questionPrompt));
  GtkWidget *e =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->errorPrompt));
  GtkWidget *i =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->infoPrompt));
  if (question && q) {
    set_text(q, question);
    gtk_widget_set_visible(q, TRUE);
  }
  if (error && e) {
    set_text(e, error);
    gtk_widget_set_visible(e, TRUE);
  }
  if (info && i) {
    set_text(i, info);
    gtk_widget_set_visible(i, TRUE);
  }
  gtk_window_present(GTK_WINDOW(ctx->window));
}

void window_empty(struct Window *ctx) {}

struct Window *create_window(GdkMonitor *monitor) {
  struct Window *w = calloc(1, sizeof(struct Window));
  if (w == NULL) {
    fprintf(stderr, "failed to allocate Window instance\n");
    exit(1);
  }
  w->monitor = monitor;

  return w;
}
