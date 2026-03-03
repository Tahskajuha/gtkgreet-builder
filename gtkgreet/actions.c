#define _POSIX_C_SOURCE 200809L

#include <assert.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "actions.h"
#include "gtkgreet.h"
#include "proto.h"
#include "uimodel.h"
#include "window.h"

static void append_string(char **dest, const char *suffix) {
  if (suffix == NULL)
    return;
  if (*dest == NULL) {
    *dest = g_strdup(suffix);
  } else {
    char *tmp = g_strconcat(*dest, "\n", suffix, NULL);
    g_free(*dest);
    *dest = tmp;
  }
}

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
    switch (resp.body.response_auth_message.auth_message_type) {
    case auth_message_type_visible:
    case auth_message_type_secret: {
      gtkgreet->question_type = QuestionTypePamPrompt;
      g_free(gtkgreet->question);
      gtkgreet->question =
          g_strdup(resp.body.response_auth_message.auth_message);
      gtkgreet_setup_question(gtkgreet);

      break;
    }
    case auth_message_type_info: {
      append_string(&gtkgreet->info,
                    resp.body.response_auth_message.auth_message);
      struct request req = {
          .request_type = request_type_post_auth_message_response,
      };
      req.body.request_post_auth_message_response.response[0] = '\0';
      handle_response(roundtrip(req), 0);

      break;
    }
    case auth_message_type_error: {
      append_string(&gtkgreet->error,
                    resp.body.response_auth_message.auth_message);
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
