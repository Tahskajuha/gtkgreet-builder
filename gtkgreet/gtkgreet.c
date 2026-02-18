#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>

#include <glib/gi18n.h>

#include "gtkgreet.h"
#include "window.h"

void gtkgreet_setup_question(struct GtkGreet *gtkgreet, enum QuestionType type,
                             char *question, char *error) {
  if (gtkgreet->question != NULL) {
    free(gtkgreet->question);
    gtkgreet->question = NULL;
  }
  if (gtkgreet->error != NULL) {
    free(gtkgreet->error);
    gtkgreet->error = NULL;
  }
  gtkgreet->question_type = type;
  if (question != NULL)
    gtkgreet->question = strdup(question);
  if (error != NULL)
    gtkgreet->error = strdup(error);
  gtkgreet->question_cnt += 1;
  window_configure(gtkgreet->window);
}

struct GtkGreet *create_gtkgreet() {
  gtkgreet = calloc(1, sizeof(struct GtkGreet));
  gtkgreet->app =
      gtk_application_new("wtf.kl.gtkgreet", G_APPLICATION_DEFAULT_FLAGS);
  gtkgreet->question_cnt = 1;
  gtkgreet->commandList = gtk_string_list_new(NULL);
  return gtkgreet;
}

void gtkgreet_activate(struct GtkGreet *gtkgreet) {
  gtkgreet_setup_question(gtkgreet, QuestionTypeInitial,
                          gtkgreet_get_initial_question(), NULL);
}

void gtkgreet_destroy(struct GtkGreet *gtkgreet) {
  if (gtkgreet->question != NULL) {
    free(gtkgreet->question);
    gtkgreet->question = NULL;
  }
  if (gtkgreet->error != NULL) {
    free(gtkgreet->error);
    gtkgreet->error = NULL;
  }
  if (gtkgreet->commandList != NULL) {
    g_object_unref(gtkgreet->commandList);
  }

  g_object_unref(gtkgreet->app);

  free(gtkgreet);
}

char *gtkgreet_get_initial_question() { return _("Username:"); }
