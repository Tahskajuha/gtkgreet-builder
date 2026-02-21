#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "actions.h"
#include "uimodel.h"

struct UiModel *create_uimodel() {
  uimodel = calloc(1, sizeof(struct UiModel));
  return uimodel;
}
