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

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state,
                               gpointer data) {
  struct Window *ctx = data;

  if ((keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) &&
      !(state & (GDK_CONTROL_MASK | GDK_ALT_MASK | GDK_SHIFT_MASK))) {
    action_answer_question(NULL, ctx);
    return TRUE;
  }

  if (keyval == GDK_KEY_Escape) {
    action_cancel_question(NULL, ctx);
    return TRUE;
  }

  return FALSE;
}

static const Widget widget_template[ROLE_COUNT] = {
    [INITIAL_ANSWER] =
        {
            .section = "core",
            .field = "initial_answer",
        },
    [PAM_PROMPT_ANSWER] =
        {
            .section = "core",
            .field = "pam_prompt_answer",
        },
    [READ_COMMAND] =
        {
            .section = "core",
            .field = "read_command",
        },
    [COMMAND_LIST] =
        {
            .section = "optional",
            .field = "command_list",
        },
    [POWEROFF] =
        {
            .section = "optional",
            .field = "poweroff",
        },
    [SUSPEND] =
        {
            .section = "optional",
            .field = "suspend",
        },
    [REBOOT] =
        {
            .section = "optional",
            .field = "reboot",
        },
    [HIBERNATE] =
        {
            .section = "optional",
            .field = "hibernate",
        },
    [CANCEL] =
        {
            .section = "optional",
            .field = "cancel",
        },
    [ERROR_PROMPT] =
        {
            .section = "optional",
            .field = "error_prompt",
        },
    [INFO_PROMPT] =
        {
            .section = "optional",
            .field = "info_prompt",
        },
    [QUESTION_PROMPT] =
        {
            .section = "optional",
            .field = "question_prompt",
        },
    [SUBMIT] =
        {
            .section = "optional",
            .field = "submit",
        },
};

struct UiModel *create_uimodel() {
  uimodel = calloc(1, sizeof(struct UiModel));
  uimodel->widgets = calloc(ROLE_COUNT, sizeof(Widget));
  memcpy(uimodel->widgets, widget_template, sizeof(widget_template));
  uimodel->initial_state = g_ptr_array_new_with_free_func(g_free);
  uimodel->pam_state = g_ptr_array_new_with_free_func(g_free);
  return uimodel;
}

void bind_actions(struct Window *ctx) {
  GtkEventController *key = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key, GTK_PHASE_CAPTURE);
  g_signal_connect(key, "key-pressed", G_CALLBACK(on_key_pressed), ctx);
  gtk_widget_add_controller(ctx->window, key);

  if (uimodel->widgets[SUBMIT].w) {
    g_signal_connect(uimodel->widgets[SUBMIT].w, "clicked",
                     G_CALLBACK(action_answer_question), ctx);
  }

  if (uimodel->widgets[CANCEL].w) {
    g_signal_connect(uimodel->widgets[CANCEL].w, "clicked",
                     G_CALLBACK(action_cancel_question), ctx);
  }

  if (uimodel->widgets[COMMAND_LIST].w) {
    if (GTK_IS_DROP_DOWN(uimodel->widgets[COMMAND_LIST].w)) {
      gtk_drop_down_set_model(GTK_DROP_DOWN(uimodel->widgets[COMMAND_LIST].w),
                              G_LIST_MODEL(gtkgreet->commandList));
    } else if (GTK_IS_LIST_VIEW(uimodel->widgets[COMMAND_LIST].w)) {
      GtkSingleSelection *sel =
          gtk_single_selection_new(G_LIST_MODEL(gtkgreet->commandList));
      GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
      g_signal_connect(factory, "setup", G_CALLBACK(list_item_setup), NULL);
      g_signal_connect(factory, "bind", G_CALLBACK(list_item_bind), NULL);

      gtk_list_view_set_model(GTK_LIST_VIEW(uimodel->widgets[COMMAND_LIST].w),
                              GTK_SELECTION_MODEL(sel));
      gtk_list_view_set_factory(GTK_LIST_VIEW(uimodel->widgets[COMMAND_LIST].w),
                                GTK_LIST_ITEM_FACTORY(factory));
    }
  }

  if (uimodel->widgets[POWEROFF].w) {
    g_signal_connect(uimodel->widgets[POWEROFF].w, "clicked",
                     G_CALLBACK(action_poweroff), ctx);
  }

  if (uimodel->widgets[REBOOT].w) {
    g_signal_connect(uimodel->widgets[REBOOT].w, "clicked",
                     G_CALLBACK(action_reboot), ctx);
  }

  if (uimodel->widgets[SUSPEND].w) {
    g_signal_connect(uimodel->widgets[SUSPEND].w, "clicked",
                     G_CALLBACK(action_suspend), ctx);
  }

  if (uimodel->widgets[HIBERNATE].w) {
    g_signal_connect(uimodel->widgets[HIBERNATE].w, "clicked",
                     G_CALLBACK(action_hibernate), ctx);
  }

  g_signal_connect(gtkgreet->window->window, "destroy",
                   G_CALLBACK(window_empty), gtkgreet->window);
}
