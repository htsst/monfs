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

#ifndef MONFS_H_
#define MONFS_H_

#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

int monfs_monitor_init(const char *);
void monfs_monitor_destroy();
int monfs_monitor_open(pid_t, uint64_t, const char *);
int monfs_monitor_read(uint64_t, ssize_t, struct timeval *);
int monfs_monitor_write(uint64_t, ssize_t, struct timeval *);
int monfs_monitor_close(uint64_t, const char *);

enum monfs_errcode {
  MONFS_OK,
  MONFS_OK_NOT_MONITORED,
  MONFS_ERR_NO_MEMORY,
  /* config error */
  MONFS_ERR_CONF_OPEN,
  MONFS_ERR_CONF_PARSE,
  /* apt error */
  MONFS_ERR_NO_ENTRY_SPACE,
  MONFS_ERR_NO_ENTRY,
  MONFS_ERR_ENTRY_NOT_PURGED,
  /* apq error */
  MONFS_ERR_APQ_INIT,
  MONFS_ERR_APQ_ALLOC,
  MONFS_ERR_APQ_ENQ,
  MONFS_ERR_APQ_DEQ,
  MONFS_ERR_APQ_WAIT,
  /* logger error */
  MONFS_ERR_LOGGER_INIT,
  /* db error */
  MONFS_ERR_DB_INIT,
  MONFS_ERR_DB_OPEN,
  MONFS_ERR_DB_EXEC,

  MONFS_ERR_NUMBER
};

#endif /* MONFS_H_ */
