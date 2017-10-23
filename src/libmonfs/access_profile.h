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

#ifndef ACCESS_PROFILE_H_
#define ACCESS_PROFILE_H_

struct access_profile;

void ap_set_path(struct access_profile *, const char *);
void ap_set_caller(struct access_profile *, pid_t, const char *);
void ap_set_open(struct access_profile *);
void ap_set_close(struct access_profile *);
void ap_update_read(struct access_profile *, ssize_t, struct timeval *);
void ap_update_write(struct access_profile *, ssize_t, struct timeval *);
void ap_set_hostname(struct access_profile *, const char *);

char * ap_get_path(struct access_profile *);
pid_t ap_get_pid(struct access_profile *);
char * ap_get_caller_path(struct access_profile *);
unsigned long ap_get_time_stamp(struct access_profile *);
unsigned long long ap_get_r_size(struct access_profile *);
unsigned long ap_get_r_sec(struct access_profile *);
unsigned long ap_get_r_usec(struct access_profile *);
unsigned long long ap_get_w_size(struct access_profile *);
unsigned long ap_get_w_sec(struct access_profile *);
unsigned long ap_get_w_usec(struct access_profile *);
char * ap_get_hostname(struct access_profile *);

int ap_alloc(struct access_profile **);
void ap_free(void *);

#endif /* ACCESS_PROFILE_H_ */
