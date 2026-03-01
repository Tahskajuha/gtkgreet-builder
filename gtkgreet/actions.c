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
    strncpy(req.body.request_start_session.cmd, gtkgreet->selected_command,
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
      gtkgreet_setup_question(gtkgreet, QuestionTypeInitial,
                              gtkgreet_get_initial_question(), error, NULL);
      break;
    }

    gtkgreet_setup_question(
        gtkgreet,
        (enum QuestionType)resp.body.response_auth_message.auth_message_type,
        resp.body.response_auth_message.auth_message, NULL, NULL);
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
    gtkgreet_setup_question(gtkgreet, QuestionTypeInitial,
                            gtkgreet_get_initial_question(), error, NULL);
    break;
  }
  }
}

static char *get_text(GtkWidget *w) {
  if (!w)
    return NULL;
  if (GTK_IS_EDITABLE(w))
    return g_strdup(gtk_editable_get_text(GTK_EDITABLE(w)));
  if (GTK_IS_DROP_DOWN(w)) {
    GtkDropDown *dd = GTK_DROP_DOWN(w);
    GObject *item = gtk_drop_down_get_selected_item(dd);
    if (!item)
      return NULL;
    return g_strdup(gtk_string_object_get_string(GTK_STRING_OBJECT(item)));
  }
  return NULL;
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
    if (gtk_builder_get_object(ctx->builder, uimodel->initialAnswer) != NULL) {
      char *text = get_text(GTK_WIDGET(
          gtk_builder_get_object(ctx->builder, uimodel->initialAnswer)));
      strncpy(req.body.request_create_session.username, text,
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
    if (gtk_builder_get_object(ctx->builder, uimodel->pamPromptAnswer) !=
        NULL) {
      char *text = get_text(GTK_WIDGET(
          gtk_builder_get_object(ctx->builder, uimodel->pamPromptAnswer)));
      strncpy(req.body.request_create_session.username, text,
              sizeof(req.body.request_create_session.username));
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

  gtkgreet_setup_question(gtkgreet, QuestionTypeInitial,
                          gtkgreet_get_initial_question(), NULL, NULL);
}
