#ifndef _FBWIDGETS_H
#define _FBWIDGETS_H

#include <gtk/gtk.h>

GtkWidget *create_fbInitialAnswer();
GtkWidget *create_fbPamPromptAnswer();
GtkWidget *create_fbReadCommand();
GtkWidget *create_fbRoot();
void push_widget_into_root(GtkBox *w);

#endif
