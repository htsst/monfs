/** This file is part of MonFS. **
 * 
 * MonFS: File system for monitoring file I/O operations
 * 
 * Copyright (C) 2010 Hitoshi Sato <hitoshi.sato@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef MONFS_CONFIG
#define MONFS_CONFIG "/etc/monfs.conf"
#endif

void monfs_config_set_filename(char *);
int monfs_config_read();
void monfs_config_set_db_path(char *);
char * monfs_config_get_db_path();
void monfs_config_free_db_path();

#endif /* CONFIG_H_ */

