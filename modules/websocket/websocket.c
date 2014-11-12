/*
 * Copyright (c) 2014 Laszlo Meszaros <lacienator@gmail.com>
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
static void
websocket_dd_disconnect(LogThrDestDriver *destination)
{
}

static worker_insert_result_t
websocket_worker_insert(LogThrDestDriver *destination, LogMessage *msg)
{
  return NULL;
}
static void
websocket_worker_thread_init(LogThrDestDriver *destination)
{
}

gboolean
websocket_dd_init(LogPipe *destination)
{
  return TRUE:
}

static void
websocket_dd_free(LogPipe *destination)
{
  log_threaded_dest_driver_free(destination);
}

LogDriver *
websocket_dd_new(GlobalConfig *cfg)
{
  return NULL;
}

