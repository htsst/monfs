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

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <monfs.h>
#include "access_profile.h"
#include "access_profile_queue.h"
#include "logger.h"
#include "error.h"


static sqlite3 *log = NULL;
static pthread_t logger;
static pthread_attr_t logger_attr;

#include <stdio.h>

static void *
do_logging(void *args) {
  int res;
  char *e, *sql;
  struct access_profile *ap;
	
  for (;;) {
    res = apq_wait();
    if (res != MONFS_OK)
      monfs_err_msg(res, NULL);

    if (sqlite3_exec(log, "BEGIN", NULL, NULL, &e) != SQLITE_OK)
      continue;
    
    res = apq_dequeue(&ap);
    if (res != MONFS_OK) {
      monfs_err_msg(res, NULL);
      goto commit;
    }

    sql = sqlite3_mprintf("INSERT INTO trace VALUES('%ld', '%ld', '%s', '%s', '%llu', '%ld', '%ld', '%llu', '%ld', '%ld', '%s')", 
			  ap_get_time_stamp(ap), ap_get_pid(ap), ap_get_caller_path(ap), ap_get_path(ap), 
    			  ap_get_r_size(ap), ap_get_r_sec(ap), ap_get_r_usec(ap), 
    			  ap_get_w_size(ap), ap_get_w_sec(ap), ap_get_w_usec(ap), 
    			  ap_get_hostname(ap));
    if (sql == NULL) {
      monfs_err_msg(MONFS_ERR_NO_MEMORY, NULL);
      goto commit;
    }
		
    if (sqlite3_exec(log, sql, NULL, NULL, &e) != SQLITE_OK)
      monfs_err_msg(MONFS_ERR_DB_EXEC, NULL);

    sqlite3_free(sql);
		
  commit:
    switch(sqlite3_exec(log, "COMMIT", NULL, NULL, &e)) {
    case SQLITE_OK:
      /* do nothing */
      break;
    case SQLITE_BUSY:
      goto commit;
      break;
    default:
      monfs_err_msg(MONFS_ERR_DB_EXEC, NULL);
      break;
    }
    ap_free(ap);
  }
  /* DO NOT REACHED */
}

static int
db_init(const char *db_path) {
  char *e, *sql;
  int res;

  /** FIXME **/
  unlink(db_path);
	
  if (log != NULL)
    return MONFS_ERR_DB_INIT;
	
  if (sqlite3_open(db_path, &log) != SQLITE_OK)
    return MONFS_ERR_DB_OPEN;
	
  sql = sqlite3_mprintf("CREATE TABLE trace (time_stamp, pid, caller_path, path, r_size, r_sec, r_usec, w_size, w_sec, w_usec, hostname)");
  if (sql == NULL) {
    res = MONFS_ERR_NO_MEMORY;
    goto error;
  }
	
  if (sqlite3_exec(log, sql, NULL, NULL, &e) == SQLITE_OK) 
    res = MONFS_OK;
  else
    res = MONFS_ERR_DB_EXEC;

  sqlite3_free(sql);

  if (res != MONFS_OK)
    goto error;
	
  return MONFS_OK;
	
 error:
  sqlite3_close(log);
  return res;
}

static void
db_destroy() {
  if (log == NULL)
    return;
	
  sqlite3_close(log);
  log = NULL;
}

int
log_ap(struct access_profile *ap)
{
  return apq_enqueue(ap);
}

int
start_logger(const char *db_path)
{
  int res;

  res = db_init(db_path);
  if (res != MONFS_OK)
    return res;

  res = apq_init();
  if (res != MONFS_OK)
    goto apq_init_error;
	
  res = pthread_attr_init(&logger_attr);
  if (res != 0) {
    res = MONFS_ERR_LOGGER_INIT;
    goto thread_attr_init_error;
  }

  res = pthread_attr_setdetachstate(&logger_attr, PTHREAD_CREATE_DETACHED);
  if (res != 0) {
    res = MONFS_ERR_LOGGER_INIT;
    goto thread_error;
  }

  res = pthread_create(&logger, NULL, do_logging, NULL);
  if (res != 0) {
    res = MONFS_ERR_LOGGER_INIT;
    goto thread_error;
  }
	
  return MONFS_OK;
	
 thread_error:
  pthread_attr_destroy(&logger_attr);
 thread_attr_init_error:
  apq_destroy();
 apq_init_error:
  db_destroy();
  return res;
}

void
stop_logger()
{
  pthread_cancel(logger);
  pthread_attr_destroy(&logger_attr);
  apq_destroy();
  db_destroy();
}
