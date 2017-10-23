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
#include <string.h>
#include <monfs.h>

/*
 * I/O Profile
 */
struct io_profile {
  unsigned long long size;
  struct timeval time;
};

static void
iop_clear(struct io_profile *iop)
{
  iop->size = 0ULL;
  timerclear(&(iop->time));
}

static void
iop_update(struct io_profile *iop, ssize_t size, struct timeval *time)
{
  iop->size += (unsigned long long) size;
  timeradd(&(iop->time), time, &(iop->time));
}

/*
 * Access Profile
 */
struct access_profile {
  char *path;
  pid_t pid;
  char *caller_path;
  struct timeval open, close;
  struct io_profile read, write;
  char *hostname;
};

static void
ap_clear(struct access_profile *ap)
{
  ap->path = NULL;
  ap->pid = 0;
  ap->caller_path = NULL;
  timerclear(&(ap->open));
  iop_clear(&(ap->read));
  iop_clear(&(ap->write));
  ap->hostname = NULL;
}

void
ap_set_path(struct access_profile *ap, const char *path)
{
  if (path == NULL)
    ap->path = strdup("");
  else
    ap->path = strdup(path);
}

void
ap_set_caller(struct access_profile *ap, pid_t pid, const char *caller_path)
{
  ap->pid = pid;
  ap->caller_path = strdup(caller_path);
}

void
ap_set_open(struct access_profile *ap)
{
  gettimeofday(&(ap->open), NULL);
}

void
ap_set_close(struct access_profile *ap)
{
  gettimeofday(&(ap->close), NULL);
}

void
ap_update_read(struct access_profile *ap, ssize_t size, struct timeval *time)
{
  iop_update(&(ap->read), size, time);
}

void
ap_update_write(struct access_profile *ap, ssize_t size, struct timeval *time)
{
  iop_update(&(ap->write), size, time);
}

void
ap_set_hostname(struct access_profile *ap, const char *hostname)
{
  ap->hostname = strdup(hostname);
}

static unsigned long
get_sec(const struct timeval *t)
{
  return (unsigned long)(t->tv_sec);
}

static unsigned long
get_usec(const struct timeval *t)
{
  return (unsigned long)(t->tv_usec);
}

char *
ap_get_path(struct access_profile *ap)
{
  return ap->path;
}

pid_t
ap_get_pid(struct access_profile *ap)
{
  return ap->pid;
}

char *
ap_get_caller_path(struct access_profile *ap)
{
  return ap->caller_path;
}

unsigned long
ap_get_time_stamp(struct access_profile *ap)
{
  return get_sec(&(ap->open));
}

unsigned long long
ap_get_r_size(struct access_profile *ap)
{
  return ap->read.size;
}

unsigned long
ap_get_r_sec(struct access_profile *ap)
{
  return get_sec(&(ap->read.time));
}

unsigned long
ap_get_r_usec(struct access_profile *ap)
{
  return get_usec(&(ap->read.time));
}

unsigned long long
ap_get_w_size(struct access_profile *ap)
{
  return ap->write.size;
}

unsigned long
ap_get_w_sec(struct access_profile *ap)
{
  return get_sec(&(ap->write.time));
}

unsigned long
ap_get_w_usec(struct access_profile *ap)
{
  return get_usec(&(ap->write.time));
}

char *
ap_get_hostname(struct access_profile *ap)
{
  return ap->hostname;
}

int
ap_alloc(struct access_profile **app)
{
  struct access_profile *ap;

  ap = malloc(sizeof(*ap));
  if (ap == NULL) {
    *app = NULL;
    return MONFS_ERR_NO_MEMORY;
  }

  ap_clear(ap);

  *app = ap;

  return MONFS_OK;
}

void
ap_free(void *args)
{
  struct access_profile *ap = (struct access_profile *)args;

  if (ap == NULL)
    return;

  if (ap->path != NULL)
    free(ap->path);

  if (ap->caller_path != NULL)
    free(ap->caller_path);

  if (ap->hostname != NULL)
    free(ap->hostname);

  free(ap);
}
