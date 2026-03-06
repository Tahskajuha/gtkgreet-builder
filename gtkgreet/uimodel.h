#ifndef _UIMODEL_H
#define _UIMODEL_H

#include <glib/gi18n.h>
#include <window.h>

struct UiModel {
  char *readCommand;
  char *submit;
  char *cancel;
  char *errorPrompt;
  char *infoPrompt;
  char *initialAnswer;
  char *pamPromptAnswer;

  char *commandList;
  char *poweroff;
  char *suspend;
  char *reboot;

  GPtrArray *initial_state;
  GPtrArray *pam_state;
};

extern struct UiModel *uimodel;

struct UiModel *create_uimodel();
void bind_widgets(struct Window *ctx);

#endif
