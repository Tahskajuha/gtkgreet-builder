#ifndef _CONFIG_H
#define _CONFIG_H

#include <gtk/gtk.h>

struct CriticalRoleIds {
  char *root;
  char *readCommand;
  char *initialAnswer;
  char *pamPromptAnswer;
};

void resolve_config(const char *config);

#endif
