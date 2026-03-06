#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "actions.h"
#include "uimodel.h"
#include "window.h"

struct UiModel *create_uimodel() {
  uimodel = calloc(1, sizeof(struct UiModel));
  uimodel->initial_state = g_ptr_array_new_with_free_func(g_free);
  uimodel->pam_state = g_ptr_array_new_with_free_func(g_free);
  return uimodel;
}

void bind_widgets(struct Window *ctx) {
  GtkWidget *submit =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->submit));
  g_signal_connect(submit, "clicked", G_CALLBACK(action_answer_question), ctx);
  GtkWidget *cancel =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->cancel));
  g_signal_connect(cancel, "clicked", G_CALLBACK(action_cancel_question), ctx);
}
