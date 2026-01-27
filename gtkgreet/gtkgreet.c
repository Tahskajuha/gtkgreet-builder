#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>

#include <glib/gi18n.h>

#include "window.h"
#include "gtkgreet.h"

struct Window* gtkgreet_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *window) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->window == window) {
            return ctx;
        }
    }
    return NULL;
}

struct Window* gtkgreet_window_by_monitor(struct GtkGreet *gtkgreet, GdkMonitor *monitor) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->monitor == monitor) {
            return ctx;
        }
    }
    return NULL;
}

void gtkgreet_remove_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *widget) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->window == widget) {
            if (gtkgreet->focused_window) {
                gtkgreet->focused_window = NULL;
            }
            free(ctx);
            g_array_remove_index_fast(gtkgreet->windows, idx);
            return;
        }
    }
}

void gtkgreet_focus_window(struct GtkGreet *gtkgreet, struct Window* win) {
    struct Window *old = gtkgreet->focused_window;
    gtkgreet->focused_window = win;
    window_swap_focus(win, old);
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        window_configure(ctx);
    }
}

void gtkgreet_setup_question(struct GtkGreet *gtkgreet, enum QuestionType type, char* question, char* error) {
    if (gtkgreet->question != NULL) {
        free(gtkgreet->question);
        gtkgreet->question = NULL;
    }
    if (gtkgreet->error != NULL) {
        free(gtkgreet->error);
        gtkgreet->error = NULL;
    }
    gtkgreet->question_type = type;
    if (question != NULL)
        gtkgreet->question = strdup(question);
    if (error != NULL)
        gtkgreet->error = strdup(error);
    gtkgreet->question_cnt += 1;
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        window_configure(ctx);
    }
}

struct GtkGreet* create_gtkgreet() {
    gtkgreet = calloc(1, sizeof(struct GtkGreet));
    gtkgreet->app = gtk_application_new("wtf.kl.gtkgreet", G_APPLICATION_DEFAULT_FLAGS);
    gtkgreet->windows = g_array_new(FALSE, TRUE, sizeof(struct Window*));
    gtkgreet->question_cnt = 1;
    return gtkgreet;
}

void gtkgreet_activate(struct GtkGreet *gtkgreet) {
    gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, gtkgreet_get_initial_question(), NULL);
}

void gtkgreet_destroy(struct GtkGreet *gtkgreet) {
    if (gtkgreet->question != NULL) {
        free(gtkgreet->question);
        gtkgreet->question = NULL;
    }
    if (gtkgreet->error != NULL) {
        free(gtkgreet->error);
        gtkgreet->error = NULL;
    }

    g_object_unref(gtkgreet->app);
    g_array_unref(gtkgreet->windows);

    free(gtkgreet);
}

char* gtkgreet_get_initial_question() {
    return _("Username:");
}
