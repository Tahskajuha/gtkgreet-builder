#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "actions.h"
#include "uimodel.h"

struct UiModel *create_uimodel() {
  uimodel = calloc(1, sizeof(struct UiModel));
  uimodel->initial_state = g_ptr_array_new_with_free_func(g_free);
  uimodel->pam_state = g_ptr_array_new_with_free_func(g_free);
  return uimodel;
}
