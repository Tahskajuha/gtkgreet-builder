#ifndef _GTKGREET_H
#define _GTKGREET_H

#include <gtk/gtk.h>

enum QuestionType {
  QuestionTypeInitial = 0,
  QuestionTypePamPrompt = 1,
};

// Defined in window.h
struct Window;

struct GtkGreet {
  GtkApplication *app;
  struct Window *window;
  GdkPixbuf *background;

#ifdef LAYER_SHELL
  gboolean use_layer_shell;
#endif
  GtkStringList *commandList;

  char *selected_command;
  enum QuestionType question_type;
  char *question;
  char *info;
  char *error;
  int question_cnt;
};

extern struct GtkGreet *gtkgreet;

void gtkgreet_setup_question(struct GtkGreet *gtkgreet, enum QuestionType type,
                             char *question, char *error);
struct GtkGreet *create_gtkgreet();
void gtkgreet_activate(struct GtkGreet *gtkgreet);
void gtkgreet_destroy(struct GtkGreet *gtkgreet);
char *gtkgreet_get_initial_question();

#endif
