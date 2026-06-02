#ifndef _UIMODEL_H
#define _UIMODEL_H

#include <glib/gi18n.h>
#include <window.h>

struct UiModel {
  char *readCommand;
  char *initialAnswer;
  char *pamPromptAnswer;

  char *commandList;
  char *poweroff;
  char *suspend;
  char *reboot;
  char *hibernate;
  char *cancel;
  char *errorPrompt;
  char *infoPrompt;
  char *questionPrompt;
  char *submit;

  GPtrArray *initial_state;
  GPtrArray *pam_state;

  GtkWidget *fbInitialPrompt;
  GtkWidget *fbPamPromptAnswer;
  GtkWidget *fbReadCommand;
  GtkWidget *fbRoot;
};

extern struct UiModel *uimodel;

struct UiModel *create_uimodel();
void bind_widgets(struct Window *ctx);

#endif
