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
#include <monfs.h>
#include "access_profile.h"
#include "queue.h"

static struct queue *apq = NULL;

int
apq_init()
{
  if (apq != NULL)
    return MONFS_ERR_APQ_INIT;

  if (queue_alloc(&apq) != 0)
    return MONFS_ERR_APQ_ALLOC;

  return MONFS_OK;
}

void
apq_destroy()
{
  queue_free(apq, ap_free);
}

int
apq_enqueue(struct access_profile *ap)
{
  if (enqueue(apq, (void *)ap) != 0)
    return MONFS_ERR_APQ_ENQ;

  return MONFS_OK;
}

int
apq_dequeue(struct access_profile **app)
{
  void *ap;

  if (dequeue(apq, &ap) != 0) {
    *app = NULL;
    return MONFS_ERR_APQ_DEQ;
  } 

  *app = (struct access_profile *)ap;
  return MONFS_OK;
}

int
apq_wait() {
  if (queue_wait(apq) != 0)
    return MONFS_ERR_APQ_WAIT;

  return MONFS_OK;
}
