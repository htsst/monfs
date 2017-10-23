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

#include <stdio.h>
#include <monfs.h>

static const char *monfs_err_str[MONFS_ERR_NUMBER] = {
  "success",
  "not monitored",
  "no memory",
  /*config error */
  "can't open config file",
  "can't parse",
  /* apt error */
  "no entry space",
  "no entry",
  "entry not purged",
  /* apq error */
  "queue initialization failed",
  "queue allocation failed",
  "can't enqueue",
  "can't dequeue",
  "queue wait failed",
  /* logger error */
  "logger initialization failed",
  /* db error */
  "db initialization failed",
  "can't open db",
  "can't execute db operation"
};

#if 0
void monfs_msg(const char  *str)
{
  if (str == NULL)
    str = "";
  fprintf(stderr, "MONFS : %s\n", str);
}
#endif

void monfs_err_msg(int no, const char *str)
{
  if (str == NULL)    
    str = "";
  fprintf(stderr, "MONFS : %s %s\n", monfs_err_str[no], str);
}
