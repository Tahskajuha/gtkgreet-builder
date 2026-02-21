#define _POSIX_C_SOURCE 200809L

#include <assert.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "gtkgreet.h"
#include "window.h"

#include <gtk-layer-shell.h>

#ifdef LAYER_SHELL

static void window_setup_layershell(struct Window *ctx) {
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

void window_setup_question(struct Window *ctx, enum QuestionType type,
                           char *question, char *error, char *info) {}

static void window_empty(struct Window *ctx) {}

struct Window *create_window(GdkMonitor *monitor) {
  struct Window *w = calloc(1, sizeof(struct Window));
  if (w == NULL) {
    fprintf(stderr, "failed to allocate Window instance\n");
    exit(1);
  }
  w->monitor = monitor;

  w->window = gtk_application_window_new(gtkgreet->app);
  g_signal_connect(w->window, "destroy", G_CALLBACK(window_empty), NULL);
  gtk_window_set_title(GTK_WINDOW(w->window), "Greeter");
  gtk_window_set_default_size(GTK_WINDOW(w->window), 200, 200);

  return w;
}
