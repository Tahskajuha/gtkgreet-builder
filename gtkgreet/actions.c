#define _POSIX_C_SOURCE 200809L

#include <assert.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "actions.h"
#include "gtkgreet.h"
#include "proto.h"
#include "uimodel.h"
#include "window.h"

static void handle_response(struct response resp, int start_req) {
  switch (resp.response_type) {
  case response_type_success: {
    if (start_req) {
      exit(0);
    }
    struct request req = {
        .request_type = request_type_start_session,
    };
    g_strlcpy(req.body.request_start_session.cmd, gtkgreet->selected_command,
              127);
    handle_response(roundtrip(req), 1);
    break;
  }
  case response_type_auth_message: {
    if (start_req) {
      struct request req = {
          .request_type = request_type_cancel_session,
      };
      roundtrip(req);

      char *error = _("Unexpected auth question");
      gtkgreet->question_type = QuestionTypeInitial;
      g_free(gtkgreet->question);
      gtkgreet->question = g_strdup(gtkgreet_get_initial_question());
      g_free(gtkgreet->error);
      gtkgreet->error = g_strdup(error);
      g_free(gtkgreet->info);
      gtkgreet->info = NULL;
      gtkgreet_setup_question(gtkgreet);
      break;
    }
    gtkgreet->question_type = QuestionTypePamPrompt;
    switch (resp.body.response_auth_message.auth_message_type) {
    case auth_message_type_visible:
    case auth_message_type_secret: {
      g_free(gtkgreet->question);
      gtkgreet->question =
          g_strdup(resp.body.response_auth_message.auth_message);
      gtkgreet_setup_question(gtkgreet);

      break;
    }
    case auth_message_type_info: {
      g_free(gtkgreet->info);
      gtkgreet->info = g_strdup(resp.body.response_auth_message.auth_message);
      gtkgreet_setup_question(gtkgreet);
      struct request req = {
          .request_type = request_type_post_auth_message_response,
      };
      req.body.request_post_auth_message_response.response[0] = '\0';
      handle_response(roundtrip(req), 0);

      break;
    }
    case auth_message_type_error: {
      g_free(gtkgreet->error);
      gtkgreet->error = g_strdup(resp.body.response_auth_message.auth_message);
      gtkgreet_setup_question(gtkgreet);
      struct request req = {
          .request_type = request_type_post_auth_message_response,
      };
      req.body.request_post_auth_message_response.response[0] = '\0';
      handle_response(roundtrip(req), 0);

      break;
    }
    }

    break;
  }
  case response_type_roundtrip_error:
  case response_type_error: {
    struct request req = {
        .request_type = request_type_cancel_session,
    };
    roundtrip(req);

    char *error = NULL;
    if (resp.response_type == response_type_error &&
        resp.body.response_error.error_type == error_type_auth) {
      error = _("Login failed");
    } else {
      error = resp.body.response_error.description;
    }
    gtkgreet->question_type = QuestionTypeInitial;
    g_free(gtkgreet->question);
    gtkgreet->question = g_strdup(gtkgreet_get_initial_question());
    g_free(gtkgreet->error);
    gtkgreet->error = g_strdup(error);
    g_free(gtkgreet->info);
    gtkgreet->info = NULL;
    gtkgreet_setup_question(gtkgreet);
    break;
  }
  }
}

static char *get_text(GtkWidget *w) {
  if (GTK_IS_EDITABLE(w)) {
    return g_strdup(gtk_editable_get_text(GTK_EDITABLE(w)));
  } else if (GTK_IS_DROP_DOWN(w)) {
    GtkDropDown *dd = GTK_DROP_DOWN(w);
    GObject *item = gtk_drop_down_get_selected_item(dd);
    if (!item)
      return NULL;
    return g_strdup(gtk_string_object_get_string(GTK_STRING_OBJECT(item)));
  } else if (GTK_IS_LIST_VIEW(w)) {
    GtkSingleSelection *sel =
        GTK_SINGLE_SELECTION(gtk_list_view_get_model(GTK_LIST_VIEW(w)));
    guint pos = gtk_single_selection_get_selected(sel);
    if (pos != GTK_INVALID_LIST_POSITION) {
      GtkStringObject *obj = GTK_STRING_OBJECT(
          g_list_model_get_item(gtk_single_selection_get_model(sel), pos));
      const char *str = gtk_string_object_get_string(obj);
      g_object_unref(obj);
      return g_strdup(str);
    }
  }
  return NULL;
}

static void power_action(const char *method) {
  g_print("Action fired: %s", method);
  GError *error = NULL;
  GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  if (!conn) {
    g_warning("Failed to get system bus: %s",
              error ? error->message : "Unknown Error");
    if (error)
      g_error_free(error);

    return;
  }
  GVariant *result = g_dbus_connection_call_sync(
      conn, "org.freedesktop.login1", "/org/freedesktop/login1",
      "org.freedesktop.login1.Manager", method, g_variant_new("(b)", FALSE),
      NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

  if (error) {
    g_print("Error encountered while calling DBus: %s", error->message);
  }
  g_error_free(error);

  if (result) {
    g_print("Recieved result");
  }

  g_object_unref(conn);
}

void action_answer_question(GtkWidget *widget, gpointer data) {
  struct Window *ctx = data;
  switch (gtkgreet->question_type) {
  case QuestionTypeInitial: {
    if (gtkgreet->selected_command) {
      free(gtkgreet->selected_command);
      gtkgreet->selected_command = NULL;
    }
    gtkgreet->selected_command = get_text(
        GTK_WIDGET(gtk_builder_get_object(ctx->builder, uimodel->readCommand)));

    struct request req = {
        .request_type = request_type_create_session,
    };
    char *text = get_text(GTK_WIDGET(
        gtk_builder_get_object(ctx->builder, uimodel->initialAnswer)));
    if (text) {
      g_strlcpy(req.body.request_create_session.username, text,
                sizeof(req.body.request_create_session.username));
      g_free(text);
    }
    handle_response(roundtrip(req), 0);
    break;
  }
  case QuestionTypePamPrompt: {
    struct request req = {
        .request_type = request_type_post_auth_message_response,
    };
    char *text = get_text(GTK_WIDGET(
        gtk_builder_get_object(ctx->builder, uimodel->pamPromptAnswer)));
    if (text) {
      g_strlcpy(req.body.request_post_auth_message_response.response, text,
                sizeof(req.body.request_post_auth_message_response.response));
      g_free(text);
    }
    handle_response(roundtrip(req), 0);
    break;
  }
  }
}

void action_cancel_question(GtkWidget *widget, gpointer data) {
  struct request req = {
      .request_type = request_type_cancel_session,
  };
  struct response resp = roundtrip(req);
  if (resp.response_type != response_type_success) {
    exit(1);
  }

  gtkgreet->question_type = QuestionTypeInitial;
  g_free(gtkgreet->question);
  gtkgreet->question = g_strdup(gtkgreet_get_initial_question());
  g_free(gtkgreet->error);
  gtkgreet->error = NULL;
  g_free(gtkgreet->info);
  gtkgreet->info = NULL;
  gtkgreet_setup_question(gtkgreet);
}

void action_poweroff(GtkWidget *widget, gpointer data) {
  power_action("PowerOff");
}

void action_reboot(GtkWidget *widget, gpointer data) { power_action("Reboot"); }

void action_suspend(GtkWidget *widget, gpointer data) {
  power_action("Suspend");
}

void action_hibernate(GtkWidget *widget, gpointer data) {
  power_action("Hibernate");
}
