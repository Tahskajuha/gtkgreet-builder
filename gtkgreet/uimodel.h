#ifndef _UIMODEL_H
#define _UIMODEL_H

#include <glib/gi18n.h>
#include <window.h>

enum role {
  INITIAL_ANSWER = 0,
  PAM_PROMPT_ANSWER = 1,
  READ_COMMAND = 2,
  COMMAND_LIST = 3,
  POWEROFF = 4,
  SUSPEND = 5,
  REBOOT = 6,
  HIBERNATE = 7,
  CANCEL = 8,
  ERROR_PROMPT = 9,
  INFO_PROMPT = 10,
  QUESTION_PROMPT = 11,
  SUBMIT = 12,
};

struct UiModel {
  GtkWidget *root;

  GtkWidget *readCommand;
  GtkWidget *initialAnswer;
  GtkWidget *pamPromptAnswer;

  GtkWidget *commandList;
  GtkWidget *poweroff;
  GtkWidget *suspend;
  GtkWidget *reboot;
  GtkWidget *hibernate;
  GtkWidget *cancel;
  GtkWidget *errorPrompt;
  GtkWidget *infoPrompt;
  GtkWidget *questionPrompt;
  GtkWidget *submit;

  GPtrArray *initial_state;
  GPtrArray *pam_state;
};

extern struct UiModel *uimodel;

struct UiModel *create_uimodel();
void bind_actions(struct Window *ctx);

#endif
