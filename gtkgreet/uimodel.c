#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "actions.h"
#include "gtkgreet.h"
#include "uimodel.h"
#include "window.h"

static void list_item_setup(GtkListItemFactory *factory, GtkListItem *item,
                            gpointer data) {
  GtkWidget *label = gtk_label_new(NULL);
  gtk_list_item_set_child(item, label);
}

static void list_item_bind(GtkListItemFactory *factory, GtkListItem *item,
                           gpointer data) {
  GtkWidget *label = gtk_list_item_get_child(item);
  GtkStringObject *obj = GTK_STRING_OBJECT(gtk_list_item_get_item(item));
  if (!obj)
    return;
  gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(obj));
}

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
  if (cancel) {
    g_signal_connect(cancel, "clicked", G_CALLBACK(action_cancel_question),
                     ctx);
  }
  GtkWidget *commandList =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->commandList));
  if (commandList) {
    if (GTK_IS_DROP_DOWN(commandList)) {
      gtk_drop_down_set_model(GTK_DROP_DOWN(commandList),
                              G_LIST_MODEL(gtkgreet->commandList));
    } else if (GTK_IS_LIST_VIEW(commandList)) {
      GtkSingleSelection *sel =
          gtk_single_selection_new(G_LIST_MODEL(gtkgreet->commandList));
      GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
      g_signal_connect(factory, "setup", G_CALLBACK(list_item_setup), NULL);
      g_signal_connect(factory, "bind", G_CALLBACK(list_item_bind), NULL);

      gtk_list_view_set_model(GTK_LIST_VIEW(commandList),
                              GTK_SELECTION_MODEL(sel));
      gtk_list_view_set_factory(GTK_LIST_VIEW(commandList),
                                GTK_LIST_ITEM_FACTORY(factory));
    }
  }
  GtkWidget *poweroff =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->poweroff));
  if (poweroff) {
    g_signal_connect(poweroff, "clicked", G_CALLBACK(action_poweroff), ctx);
  }
  GtkWidget *reboot =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->reboot));
  if (reboot) {
    g_signal_connect(reboot, "clicked", G_CALLBACK(action_reboot), ctx);
  }
  GtkWidget *suspend =
      GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->suspend));
  if (suspend) {
    g_signal_connect(suspend, "clicked", G_CALLBACK(action_suspend), ctx);
  }
}
