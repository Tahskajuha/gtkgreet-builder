#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>

// Defined in gtkgreet.h
enum QuestionType;

struct Window {
  GdkMonitor *monitor;

  GtkWidget *window;
  GtkBuilder *builder;

#ifdef LAYER_SHELL
  gulong enter_notify_handler;
#endif

  int question_cnt;
};

struct Window *create_window(GdkMonitor *monitor);
void window_configure(struct Window *win);
void window_setup_question(struct Window *ctx, enum QuestionType type,
                           char *question, char *error);
void window_swap_focus(struct Window *win, struct Window *old);

#endif
