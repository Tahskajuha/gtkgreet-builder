#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>

#include <glib/gi18n.h>

#include "gtkgreet.h"
#include "window.h"

void gtkgreet_setup_question(struct GtkGreet *gtkgreet) {
  gtkgreet->question_cnt += 1;
  window_setup_question(gtkgreet->window, gtkgreet->question_type,
                        gtkgreet->question, gtkgreet->error, gtkgreet->info);
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
#ifdef LAYER_SHELL
  window_setup_layershell(gtkgreet->window);
#endif
  gtkgreet->question_type = QuestionTypeInitial;
  g_free(gtkgreet->question);
  gtkgreet->question = g_strdup(gtkgreet_get_initial_question());
  g_free(gtkgreet->error);
  gtkgreet->error = NULL;
  g_free(gtkgreet->info);
  gtkgreet->info = NULL;
  gtkgreet_setup_question(gtkgreet);
}

void gtkgreet_destroy(struct GtkGreet *gtkgreet) {
  g_free(gtkgreet->question);
  gtkgreet->question = NULL;
  g_free(gtkgreet->error);
  gtkgreet->error = NULL;
  if (gtkgreet->commandList != NULL) {
    g_object_unref(gtkgreet->commandList);
  }

  g_object_unref(gtkgreet->app);

  g_free(gtkgreet);
}

char *gtkgreet_get_initial_question() { return _("Username:"); }
