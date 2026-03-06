#ifndef _AUTH_QUESTION_H
#define _AUTH_QUESTION_H

#include <gtk/gtk.h>

void action_answer_question(GtkWidget *widget, gpointer data);
void action_cancel_question(GtkWidget *widget, gpointer data);
void action_poweroff(GtkWidget *widget, gpointer data);
void action_reboot(GtkWidget *widget, gpointer data);
void action_suspend(GtkWidget *widget, gpointer data);

#endif
