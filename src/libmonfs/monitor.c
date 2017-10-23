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
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <monfs.h>
#include "config.h"
#include "error.h"
#include "access_profile.h"
#include "access_profile_queue.h"
#include "logger.h"
#include "hash.h"

static struct hash_table *apt = NULL;
#define APT_SIZE 1024

static int monitored = 0;

static int
enter_into_table(uint64_t fh, struct access_profile *ap)
{
  struct hash_entry *entry;
  int created;
  struct access_profile **app;

  entry = hash_enter(apt, &fh, sizeof(uint64_t), sizeof(struct access_profile **), &created);
  if (entry == NULL || !created)
    return MONFS_ERR_NO_ENTRY_SPACE;
	
  app = (struct access_profile **)hash_entry_data(entry);
  *app = ap;
	
  return MONFS_OK;
}

static int
refer_to_table(uint64_t fh, struct access_profile **app)
{
  struct hash_entry *entry;

  entry = hash_lookup(apt, &fh, sizeof(uint64_t));
  if (entry == NULL)
    return MONFS_ERR_NO_ENTRY;
  
  *app = *((struct access_profile **)hash_entry_data(entry));

  return MONFS_OK;
}

static int
purge_from_table(uint64_t fh)
{
  if (hash_purge(apt, &fh, sizeof(uint64_t)))
    return MONFS_OK;
  else
    return MONFS_ERR_ENTRY_NOT_PURGED;
}

int
monfs_monitor_init(const char *filename)
{
  int res;
  char *db_path;
  
  if (filename != NULL) {
      monfs_config_set_db_path(filename);
  }

#if 0
  if (filename != NULL) {
    monfs_config_set_filename(filename);
  }


  res = monfs_config_read();
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    return res;
  }
#endif  

  apt = hash_table_alloc(APT_SIZE, hash_default, hash_key_equal_default);
  if (apt == NULL) {
    res = MONFS_ERR_NO_MEMORY;
    monfs_err_msg(res, NULL);
    return res;
  }

  db_path = monfs_config_get_db_path();
  res = start_logger(db_path);
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    return res;
  }
  
  monitored = 1;

  return MONFS_OK;
}

void
monfs_monitor_destroy()
{
  if (monitored) {
    stop_logger();
  // TODO table free
    monfs_config_free_db_path();
  }

}

static char *
get_caller_path(pid_t pid)
{
  int res;
  char exe[PATH_MAX + 1], caller[PATH_MAX + 1];

  memset(exe, 0, PATH_MAX+1);
  memset(caller, 0, PATH_MAX+1);

  sprintf(exe, "/proc/%d/exe", pid);

  res = readlink(exe, caller, strlen(exe)+1);
  if (res == -1)
    return NULL;

  return strdup(caller);
}

int
monfs_monitor_open(pid_t pid, uint64_t fh, const char *path)
{
  int res;
  struct access_profile *ap;
  char *caller_path;

  if (!monitored)
    return MONFS_OK_NOT_MONITORED;

  res = ap_alloc(&ap);
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    return res;
  }
  
  ap_set_path(ap, path);
  ap_set_open(ap);
  caller_path = get_caller_path(pid);
  ap_set_caller(ap, pid, caller_path);
  free(caller_path);

  res = enter_into_table(fh, ap);
  if (res != MONFS_OK) {
    ap_free(ap);
    monfs_err_msg(res, NULL);
    return res;
  }

  return MONFS_OK;
}

int
monfs_monitor_read(uint64_t fh, ssize_t size, struct timeval *time)
{
  struct access_profile *ap;
  int res;

  if (!monitored)
    return MONFS_OK_NOT_MONITORED;

  res = refer_to_table(fh, &ap);
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    return res;
  }

  ap_update_read(ap, size, time);
  return MONFS_OK;
}

int
monfs_monitor_write(uint64_t fh, ssize_t size, struct timeval *time )
{
  struct access_profile *ap;
  int res;

  if (!monitored)
    return MONFS_OK_NOT_MONITORED;

  res = refer_to_table(fh, &ap);
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    return res;
  }

  ap_update_write(ap, size, time);
  return MONFS_OK;
}

int
monfs_monitor_close(uint64_t fh, const char *hostname)
{
  struct access_profile *ap;
  int res, res_save;

  if (!monitored)
    return MONFS_OK_NOT_MONITORED;

  res = refer_to_table(fh, &ap);
  if (res != MONFS_OK) {
    monfs_err_msg(res, NULL);
    res_save = res;
    goto finish; // do not log
  }

  if (hostname == NULL) {
    res = MONFS_OK;
    goto finish; // do not log
  }

  ap_set_close(ap);
  ap_set_hostname(ap, hostname);

  res = apq_enqueue(ap);
  if (res != MONFS_OK) {
    ap_free(ap);
    monfs_err_msg(res, NULL);
  }

 finish:
  res = purge_from_table(fh);
  if (res != MONFS_OK)
    monfs_err_msg(res, NULL);

  return (res_save != 0 ? res_save : res);
}
