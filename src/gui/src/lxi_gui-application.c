/*
 * Copyright (c) 2021  Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lxi_gui-application.h"
#include "lxi_gui-window.h"
#include "lxi_gui-prefs.h"
#include "lxi_gui-config.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

struct _LxiGuiApplication
{
  GtkApplication parent_instance;
};

G_DEFINE_TYPE (LxiGuiApplication, lxi_gui_application, GTK_TYPE_APPLICATION)

LxiGuiApplication *
lxi_gui_application_new (gchar *application_id,
                         GApplicationFlags  flags)
{
  return g_object_new (LXI_GUI_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
lxi_gui_application_finalize (GObject *object)
{
  LxiGuiApplication *self = (LxiGuiApplication *)object;
  UNUSED(self);

  G_OBJECT_CLASS (lxi_gui_application_parent_class)->finalize (object);
}

static void
lxi_gui_application_activate (GApplication *app)
{
  GtkWindow *window;

  /* It's good practice to check your parameters at the beginning of the
   * function. It helps catch errors early and in development instead of
   * by your users.
   */
  g_assert (GTK_IS_APPLICATION (app));

  /* Get the current window or create one if necessary. */
  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  if (window == NULL)
    window = g_object_new (LXI_GUI_TYPE_WINDOW,
                           "application", app,
                           NULL);

  /* Ask the window manager/compositor to present the window. */
  gtk_window_present (window);
}


static void
lxi_gui_application_class_init (LxiGuiApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  object_class->finalize = lxi_gui_application_finalize;

  /*
   * We connect to the activate callback to create a window when the application
   * has been launched. Additionally, this callback notifies us when the user
   * tries to launch a "second instance" of the application. When they try
   * to do that, we'll just present any existing window.
   */
  app_class->activate = lxi_gui_application_activate;
}

static void
lxi_gui_application_show_about (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data)
{
  UNUSED(action);
  UNUSED(parameter);
  LxiGuiApplication *self = LXI_GUI_APPLICATION (user_data);
  GtkWindow *window = NULL;
  const gchar *authors[] = {"Martin Lund", NULL};
  GdkTexture *logo = gdk_texture_new_from_resource("/io/github/lxi-tools/lxi-gui/icons/lxi-tools-icon-128px.png");

  g_return_if_fail (LXI_GUI_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  gtk_show_about_dialog (window,
                         "program-name", "lxi-gui",
                         "authors", authors,
                         "version", PACKAGE_VERSION,
                         "copyright", "© 2021 Martin Lund",
                         "license-type", GTK_LICENSE_BSD_3,
                         "website", "http://lxi-tools.github.io",
                         "comments", "Program for managing LXI compatible instruments.",
                         "authors", authors,
                         "logo", logo,
                         "title", "About lxi-gui",
                         NULL);
}

static void
lxi_gui_application_show_preferences (GSimpleAction *action,
                                      GVariant      *parameter,
                                      gpointer       app)
{
  UNUSED(action);
  UNUSED(parameter);
  LxiGuiPrefs *prefs;
  GtkWindow *win;

  win = gtk_application_get_active_window (GTK_APPLICATION (app));
  prefs = lxi_gui_prefs_new (LXI_GUI_WINDOW (win));
  gtk_window_present (GTK_WINDOW (prefs));

}

static void
lxi_gui_application_init (LxiGuiApplication *self)
{
  GSimpleAction *quit_action = g_simple_action_new ("quit", NULL);
  g_signal_connect_swapped (quit_action, "activate", G_CALLBACK (g_application_quit), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (quit_action));

  GSimpleAction *preferences_action = g_simple_action_new ("preferences", NULL);
  g_signal_connect (preferences_action, "activate", G_CALLBACK (lxi_gui_application_show_preferences), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (preferences_action));

  GSimpleAction *about_action = g_simple_action_new ("about", NULL);
  g_signal_connect (about_action, "activate", G_CALLBACK (lxi_gui_application_show_about), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (about_action));

  const char *accels[] = {"<primary>q", NULL};
  gtk_application_set_accels_for_action (GTK_APPLICATION (self), "app.quit", accels);
}
