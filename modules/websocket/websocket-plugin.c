/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
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

#include <libwebsockets.h>

#include "websocket-parser.h"
#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats/stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"

extern CfgParser websocket_parser;

static Plugin websocket_plugin[] =
{
  {
    .type = LL_CONTEXT_DESTINATION,
    .name = "websocket",
    .parser = &websocket_parser,
  },
};

gboolean
websocket_module_init(GlobalConfig *cfg, CfgArgs *args)
{
  plugin_register(cfg, &websocket_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "websocket",
  .version = VERSION,
  .description = "The websocket module provides websocket destination support for syslog-ng.",
  .core_revision = "Dummy Revision",
  .plugins = &websocket_plugin,
  .plugins_len = 1,
};
