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
#include <pthread.h>
#include <monfs.h>
#include "queue.h"

struct queue_node {
  void *data;
  struct queue_node *next;
};

struct queue {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_mutex_t cond_mutex;
  struct queue_node *head;
  struct queue_node **tail_p;
};

int
queue_alloc(struct queue **queue_p)
{
  int res;
  struct queue *queue;

  queue = malloc(sizeof *queue);
  if (queue == NULL) {
    res = ENOMEM;
    goto error;
  }
	
  queue->head = NULL;
  queue->tail_p = &(queue->head);
	
  res = pthread_mutex_init(&(queue->mutex), NULL);
  if (res != 0)
    goto error_mutex;

  res = pthread_cond_init(&(queue->cond), NULL);
  if (res != 0)
    goto error_cond;
	
  res = pthread_mutex_init(&(queue->cond_mutex), NULL);
  if (res != 0)
    goto error_cond_mutex;
	
  *queue_p = queue;
	
  return 0;
	
  /** error **/
 error_cond_mutex:
  pthread_cond_destroy(&(queue->cond));
 error_cond:		
  pthread_mutex_destroy(&(queue->mutex));
 error_mutex:		
  free(queue);
 error:
  return res;
}

void
queue_free(struct queue *queue, free_func_t free_func)
{
  struct queue_node *cursor, *tmp;
	
  pthread_mutex_destroy(&(queue->cond_mutex));
  pthread_cond_destroy(&(queue->cond));
  pthread_mutex_destroy(&(queue->mutex));
	
  cursor = queue->head;
  while(cursor) {
    if (free_func)
      free_func(cursor->data);
    tmp = cursor->next;
    free(cursor);
    cursor = tmp;
  }
	
  free(queue);
}

int
enqueue(struct queue *queue, void *data)
{
  int res, res_save = 0;
  struct queue_node *node = NULL;
  int canceltype;
	
  node = malloc(sizeof(*node));
  if (node == NULL)
    return ENOMEM;
	
  node->data = data;
  node->next = NULL;
	
  res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &canceltype);
  if (res != 0)
    return res;

  /* Lock dispatch mutex, add node to dispatch queue, unlock mutex */
  res = pthread_mutex_lock(&queue->mutex);
  if (res != 0) {
    free(node);
    return res;
  }
	
  *(queue->tail_p) = node;
  queue->tail_p = &(node->next);
	
  res = pthread_mutex_unlock(&(queue->mutex));
  if (res != 0)
    return res;

  /* Signal dispatch condition */
  res = pthread_mutex_lock(&(queue->cond_mutex));
  if (res != 0)
    return res;
	
  res = pthread_cond_signal(&(queue->cond));
  if (res != 0)
    res_save = res;
	
  res = pthread_mutex_unlock(&(queue->cond_mutex));
	
  return (res_save != 0 ? res_save : res);
}

int
dequeue(struct queue *queue, void **data)
{
  int res, res_save = 0;
  struct queue_node *node;
  int canceltype;
	
  res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &canceltype);
  if (res != 0)
    return res;

  /* Lock dispatch mutex, pop node, unlock mutex */
  res = pthread_mutex_lock(&(queue->mutex));
  if (res != 0)
    return res;
	
  if (queue->head) {
    node = queue->head;
    queue->head = node->next;
    if (!queue->head)
      queue->tail_p = &(queue->head);
  } else {
    node = NULL;
  }

  res = pthread_mutex_unlock(&(queue->mutex));
  if (res != 0)
    res_save = res;
	
  res = pthread_setcanceltype(canceltype, &canceltype);

  if (node) {
    *data = node->data;
    free(node);
  }
	
  return (res_save != 0 ? res_save : res);
}

int
queue_wait(struct queue *queue)
{
  int wait_error = 0;
	
  if (pthread_mutex_lock(&(queue->cond_mutex)) != 0)
    return -1;
	
  pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock,
		       (void *)&(queue->cond_mutex));
  while (!wait_error && !queue->head) {
    if (pthread_cond_wait(&(queue->cond), &(queue->cond_mutex)) != 0)
      wait_error = -1;
  }
  pthread_cleanup_pop(1);
	
  return wait_error;
}

