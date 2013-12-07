/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gergely Nagy <algernon@balabit.hu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "journal-source.h"

#include "driver.h"
#include "logsource.h"
#include "mainloop.h"

#include <iv.h>
#include <systemd/sd-journal.h>

typedef struct _JournalSourceDriver
{
  LogSrcDriver super;
  LogSource *source;
  LogSourceOptions source_options;
} JournalSourceDriver;

typedef struct
{
  LogSource super;

  gboolean watches_running;
  sd_journal *journal;
  struct iv_fd fd;
} JournalSource;

#ifndef SCS_JOURNAL
#define SCS_JOURNAL 0
#endif

/*
 * JournalSource
 */

static void journal_source_update_watches (JournalSource *self);

static void
journal_msg_set_value (sd_journal *journal,
                       LogMessage *msg,
                       const char *field,
                       NVHandle handle)
{
  int r;
  size_t l;
  const void *d;
  ssize_t flen = strlen (field) + 1;

  r = sd_journal_get_data (journal, field, &d, &l);
  if (r < 0)
    return;

  log_msg_set_value (msg, handle, (const gchar *)(d + flen), (gssize)(l - flen));
}

static void
journal_source_triggered (void *s)
{
  JournalSource *self = (JournalSource *) s;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  int r;

  main_loop_assert_main_thread ();

  if (!log_source_free_to_send (&self->super))
    return;

  if (sd_journal_process (self->journal) != SD_JOURNAL_APPEND)
    return;

  r = sd_journal_next (self->journal);

  size_t l;
  const void *d;

  SD_JOURNAL_FOREACH_DATA(self->journal, d, l)
    printf("%.*s\n", (int) l, (const char *)d);

  printf("\n\n");

  msg = log_msg_new_empty ();
  journal_msg_set_value (self->journal, msg, "MESSAGE", LM_V_MESSAGE);

  path_options.ack_needed = FALSE;

  log_pipe_queue (&self->super.super, msg, &path_options);
}

static void
journal_source_start_watches (JournalSource *self)
{
  if (self->watches_running)
    return;

  /*
  if (self->trigger_timer.expires.tv_sec >= 0)
    iv_timer_register (&self->trigger_timer);
  self->watches_running = TRUE;
  */
}

static void
journal_source_stop_watches (JournalSource *self)
{
  if (!self->watches_running)
    return;

  /*
  if (iv_timer_registered (&self->trigger_timer))
    iv_timer_unregister (&self->trigger_timer);

  self->watches_running = FALSE;
  */
}

static void
journal_source_update_watches (JournalSource *self)
{
  if (!log_source_free_to_send (&self->super))
    {
      journal_source_stop_watches (self);
      return;
    }

  /*
  iv_validate_now ();
  journal_source_stop_watches (self);
  self->trigger_timer.expires = iv_now;
  self->trigger_timer.expires.tv_sec += self->trigger_freq;
  journal_source_start_watches (self);
  */
}

static gboolean
journal_source_init (LogPipe *s)
{
  JournalSource *self = (JournalSource *)s;

  if (!log_source_init (s))
    return FALSE;

  /*
  iv_validate_now ();
  self->trigger_timer.expires = iv_now;
  self->trigger_timer.expires.tv_sec += self->trigger_freq;
  */

  journal_source_start_watches (self);

  return TRUE;
}

static gboolean
journal_source_deinit (LogPipe *s)
{
  JournalSource *self = (JournalSource *)s;

  journal_source_stop_watches (self);

  sd_journal_close (self->journal);
  

  return log_source_deinit (s);
}


static LogSource *
journal_source_new (JournalSourceDriver *owner, LogSourceOptions *options)
{
  JournalSource *self = g_new0 (JournalSource, 1);

  log_source_init_instance (&self->super);
  log_source_set_options (&self->super, options, 0, SCS_JOURNAL,
                          owner->super.super.id, NULL, FALSE);

  sd_journal_open(&self->journal,
                  /*
                  SD_JOURNAL_LOCAL_ONLY |
                  SD_JOURNAL_RUNTIME_ONLY |
                  SD_JOURNAL_SYSTEM_ONLY | */
                  0
                  );
  IV_FD_INIT(&self->fd);
  self->fd.fd = sd_journal_get_fd(self->journal);
  self->fd.handler_in = journal_source_triggered;
  self->fd.handler_out = NULL;
  self->fd.handler_err = NULL;
  self->fd.cookie = self;

  iv_fd_register(&self->fd);

  self->super.super.init = journal_source_init;
  self->super.super.deinit = journal_source_deinit;

  return &self->super;
}

/*
 * JournalSourceDriver
 */

static gboolean
journal_sd_init (LogPipe *s)
{
  JournalSourceDriver *self = (JournalSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config (s);

  if (!log_src_driver_init_method (s))
    return FALSE;

  log_source_options_init (&self->source_options, cfg, self->super.super.group);
  self->source = journal_source_new (self, &self->source_options);

  log_pipe_append (&self->source->super, s);
  log_pipe_init (&self->source->super, cfg);

  return TRUE;
}

static gboolean
journal_sd_deinit (LogPipe *s)
{
  JournalSourceDriver *self = (JournalSourceDriver *)s;

  if (self->source)
    {
      log_pipe_deinit (&self->source->super);
      log_pipe_unref (&self->source->super);
      self->source = NULL;
    }

  return log_src_driver_deinit_method (s);
}

LogSourceOptions *
journal_sd_get_source_options (LogDriver *s)
{
  JournalSourceDriver *self = (JournalSourceDriver *)s;

  return &self->source_options;
}

LogDriver *
journal_sd_new (void)
{
  JournalSourceDriver *self = g_new0 (JournalSourceDriver, 1);

  log_src_driver_init_instance ((LogSrcDriver *)&self->super);

  self->super.super.super.init = journal_sd_init;
  self->super.super.super.deinit = journal_sd_deinit;

  log_source_options_defaults (&self->source_options);

  return (LogDriver *)self;
}
