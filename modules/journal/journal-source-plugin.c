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
#include "journal-source-parser.h"

#include "plugin.h"
#include "plugin-types.h"

extern CfgParser journal_source_parser;

static Plugin journal_source_plugin =
{
  .type = LL_CONTEXT_SOURCE,
  .name = "journal",
  .parser = &journal_source_parser,
};

gboolean
journal_module_init(GlobalConfig *cfg, CfgArgs *args G_GNUC_UNUSED)
{
  plugin_register(cfg, &journal_source_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "journal",
  .version = VERSION,
  .description = "The journal module provides a systemd-journal source.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = &journal_source_plugin,
  .plugins_len = 1,
};
