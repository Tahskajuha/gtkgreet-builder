#ifndef _UIMODEL_H
#define _UIMODEL_H

#include <glib/gi18n.h>
#include <window.h>

enum role {
  ROOT = 0,
  INITIAL_ANSWER,
  PAM_PROMPT_ANSWER,
  READ_COMMAND,
  COMMAND_LIST,
  POWEROFF,
  SUSPEND,
  REBOOT,
  HIBERNATE,
  CANCEL,
  ERROR_PROMPT,
  INFO_PROMPT,
  QUESTION_PROMPT,
  SUBMIT,

  ROLE_COUNT
};

typedef struct {
  const char *section;
  const char *field;
  GtkWidget *w;
} Widget;

struct UiModel {
  Widget *widgets;

  GPtrArray *initial_state;
  GPtrArray *pam_state;
};

extern struct UiModel *uimodel;

struct UiModel *create_uimodel();
void bind_actions(struct Window *ctx);

#endif
