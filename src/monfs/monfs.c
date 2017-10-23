
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

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef ULOCKMGR
#include <ulockmgr.h>
#endif
#include <dirent.h>
#include <monfs.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

/**
 * Default Configuration
 */
char *db_filename = NULL; //"/tmp/monfs.db";
char *monfs_root = NULL; //"";
int monitor_flag = 1;

static int monfs_root_fd = -1;

static int
is_absolute_path(const char *path)
{
  if (path && path[0] != '\0' && path[0] == '/')
    return 1;
  else 
    return 0;
}

static char *
get_relative_monfs_path(const char *path)
{
  char *relative_monfs_path;
  int len;
	
  len = strlen(path) + 2;
  relative_monfs_path = malloc(sizeof(char) * len);
  if (relative_monfs_path == NULL)
    return NULL;
	
  snprintf(relative_monfs_path, len, ".%s", path);
  relative_monfs_path[len - 1] = '\0';

  return relative_monfs_path; 
}

/** 
 *	 File operations using fuse api.
 */

/** Get file attributes. */
static int
monfs_getattr(const char *path, struct stat *stbuf)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = lstat(monfs_path, stbuf);
  if (res == -1) 
    res = -errno;

  free(monfs_path);
  return res;
}

/** Read the target of a symbolic link */
static int
monfs_readlink(const char *path, char *buf, size_t size)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = readlink(monfs_path, buf, size -1);
  if (res == -1)
    res = -errno;
  else {
    buf[res] = '\0';
    res = 0;
  }

  free(monfs_path);
  return res;
}

static int
monfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
  int res;
  char *monfs_path;

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  if (S_ISFIFO(mode))
    res = mkfifo(monfs_path, mode);
  else
    res = mknod(monfs_path, mode, rdev);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** Create a directory */
static int
monfs_mkdir(const char *path, mode_t mode)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = mkdir(monfs_path, mode);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** Remove a file */
static int 
monfs_unlink(const char *path) {
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = unlink(monfs_path);
  if (res == -1)
    res = -errno;

  free(monfs_path);
  return res;
}

/** Remove a directory */
static int
monfs_rmdir(const char *path)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = rmdir(monfs_path);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}
/** Create a symbolic link */
static int
monfs_symlink(const char *from, const char *to)
{
  int res;
  char *monfs_from, *monfs_to;
	
  monfs_to = get_relative_monfs_path(to);
  if (monfs_to == NULL)
    return -ENOMEM;
	
  res = symlink(from, monfs_to);
  if (res == -1)
    res = -errno;

  free(monfs_to);
  return res;
}

/** Rename a file */
static int
monfs_rename(const char *from , const char *to) {
  int res;
  char *monfs_from, *monfs_to;
	
  monfs_from = get_relative_monfs_path(from);
  if (monfs_from == NULL)
    return -ENOMEM;
	
  monfs_to = get_relative_monfs_path(to);
  if (monfs_to == NULL)
    return -ENOMEM;
	
  res = rename(monfs_from, monfs_to);
  if (res == -1)
    res = -errno;
	
  free(monfs_from);
  free(monfs_to);
  return res;
}

/** Create a hard link to a file */
static int
monfs_link(const char *from, const char *to) {
  int res;
  char *monfs_from, *monfs_to;
	
  monfs_from = get_relative_monfs_path(from);
  if (monfs_from == NULL)
    return -ENOMEM;
	
  monfs_to = get_relative_monfs_path(to);
  if (monfs_to == NULL)
    return -ENOMEM;
	
  res = link(monfs_from, monfs_to);
  if (res == -1)
    res = -errno;
	
  free(monfs_from);
  free(monfs_to);
  return res;
}

/** Change the permission bits of a file */
static int
monfs_chmod(const char *path, mode_t mode)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = chmod(monfs_path, mode);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** Change the owner and group of a file */
static int
monfs_chown(const char *path, uid_t uid, gid_t gid) 
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = lchown(monfs_path, uid, gid);
  if (res == -1) 
    res = -errno;

  free(monfs_path);
  return res;
}

/** Change the size of a file */
static int
monfs_truncate(const char *path, off_t size)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = truncate(monfs_path, size);
  if (res == -1) 
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** File open operation */
static int
monfs_open(const char *path, struct fuse_file_info *fi)
{
  int res;
  char *monfs_path;
    
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = open(monfs_path, fi->flags);
  if (res == -1)
    res = -errno;
  else {
    fi->fh = res;
    monfs_monitor_open(fuse_get_context()->pid, fi->fh, path);
    res = 0;
  }

  
  free(monfs_path);
  return res;
}

/** Read data from an open file */
static int
monfs_read(const char *path, char *buf, size_t size, off_t offset,
	   struct fuse_file_info *fi)
{
  int res;
  (void) path; 
  struct timeval t1, t2;

  gettimeofday(&t1, NULL);
  res = pread(fi->fh, buf, size, offset);
  gettimeofday(&t2, NULL);
  if (res == -1)
    res = -errno;
  else {
    timersub(&t2, &t1, &t2);
    monfs_monitor_read(fi->fh, res, &t2);
  }

  return res;
}

/** Write data from an open file */
static int
monfs_write(const char *path, const char *buf,
	    size_t size, off_t offset, struct fuse_file_info *fi)
{
  int res;
  (void) path; 
  struct timeval t1, t2;

  gettimeofday(&t1, NULL);
  res = pwrite(fi->fh, buf, size, offset);
  gettimeofday(&t2, NULL);
  if (res == -1)
    res = -errno;
  else {
    timersub(&t2, &t1, &t2);
    monfs_monitor_write(fi->fh, res, &t2);
  }
	
  return res;
}

/** Get file system statistics */
static int
monfs_statfs(const char* path, struct statvfs *stbuf)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = statvfs(monfs_path, stbuf);
  if (res == -1)
    res = -errno;

  free(monfs_path);
  return res;
}

/** Possibly flush cached data */
static int
monfs_flush(const char *path, struct fuse_file_info *fi)
{
  int res;
  (void) path;

  res = close(dup(fi->fh));
  if (res == -1)
    return -errno;
	
  return 0;
}

/** Release an open file */
static int
monfs_release(const char *path, struct fuse_file_info *fi)
{
  (void) path;

  monfs_monitor_close(fi->fh, "localhost");
  close(fi->fh);

  return 0;
}

/** Synchronize file contents */
static int
monfs_fsync(const char *path, int isdatasync,
	    struct fuse_file_info *fi)
{
  int res;
  (void) path;

#ifndef HAVE_FDATASYNC
  (void) isdatasync;
#else
  if (isdatasync)
    res = fdatasync(fi->fh);
  else
#endif
    res = fsync(fi->fh);
    
  if (res == -1)
    return -errno;

  return res;
}

#ifdef HAVE_SETXATTR
/** Set extended attributes */
static int
monfs_setxattr(const char *path, const char *name, const char *value,
	       size_t size, int flags)
{
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = lsetxattr(monfs_path, name, value, size, flags);
  if (res == -1)
    res = -errno;
		
  free(monfs_path);
  return res;
}

/** Get extended attributes */
static int 
monfs_getxattr(const char *path, const char *name, char *value,
	       size_t size) 
{
  int res;
  char *monfs_path;

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = lgetxattr(monfs_path, name, value, size);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** List extended attributes */
static int
monfs_listxattr(const char *path, char *list, size_t size) {
  int res;
  char *monfs_path;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = llistxattr(monfs_path, list, size);
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}

/** Remove extended attributes */
static int
monfs_removexattr(const char *path, const char *name) {
  int res;
  char *monfs_path;

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  res = lremovexattr(monfs_path, name); 
  if (res == -1)
    res = -errno;
	
  free(monfs_path);
  return res;
}
#endif /* HAVE_SETXATTR */

/** Open directory */
static int
monfs_opendir(const char *path, struct fuse_file_info *fi) {
  int res;
  char *monfs_path;
  DIR *dp;
	
  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;
	
  dp = opendir(monfs_path);
  if (dp == NULL) {
    res = -errno;
  } else {
    fi->fh = (unsigned long) dp;
    res = 0;
  }

  free(monfs_path);
  return res;
}

static inline DIR *
get_dirp(struct fuse_file_info *fi)
{
  return (DIR *) (uintptr_t) fi->fh;
}

/** Read directory */
static int
monfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
	      off_t offset, struct fuse_file_info *fi)
{
  char *monfs_path;
  DIR *dp = get_dirp(fi);
  struct dirent *de;
	
  seekdir(dp, offset);
  while ((de = readdir(dp)) != NULL) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    if (filler(buf, de->d_name, &st, telldir(dp)))
      break;
  }
	
  return 0;
}

/** Release directory */
static int
monfs_releasedir(const char *path, struct fuse_file_info *fi)
{
  DIR *dp = get_dirp(fi);
  (void) path;
  closedir(dp);
  return 0;
}

static void*
monfs_init(struct fuse_conn_info *info)
{
  fchdir(monfs_root_fd);
  close(monfs_root_fd);
  monfs_monitor_init(db_filename);
}

static void
monfs_destroy(void *private_data) 
{
  monfs_monitor_destroy();
  free(monfs_root);
  free(db_filename);
}

/** Check file access permissions */
static int
monfs_access(const char *path, int mask) {
  int res;
  char *monfs_path;

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = access(monfs_path, mask);
  if (res == -1)
    res = -errno;

  free(monfs_path);
  return res;
}

/** Create and open a file */
static int
monfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
  int res;
  char *monfs_path;

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  res = open(monfs_path, fi->flags, mode);
  if (res == -1)
    res = -errno;
  else {
    fi->fh = res;
    monfs_monitor_open(fuse_get_context()->pid, fi->fh, path);
    res = 0;
  }

  free(monfs_path);
  return res;
}

/** Change the size of an open file */
static int
monfs_ftruncate(const char *path, off_t size,
		struct fuse_file_info *fi)
{
  int res;
  (void) path;
	
  res = ftruncate(fi->fh, size);
  if (res == -1)
    return -errno;
	
  return 0;
}

/** Get attributes from an open file */
static int
monfs_fgetattr(const char *path, struct stat *stbuf,
	       struct fuse_file_info *fi)
{
  int res;
  (void) path;
	
  res = fstat(fi->fh, stbuf);
  if (res == -1)
    return -errno;
	
  return 0;
}

#if ULOCKMGR
/** Perform POSIX file locking operation */
static int
monfs_lock(const char *path, 
	   struct fuse_file_info *fi, int cmd, struct flock *lock)
{
  (void) path;

  return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner,
		     sizeof(fi->lock_owner));
}
#endif

/** Change the access and modification time of a file with nanosecond resolution */
static int
monfs_utimens(const char *path, const struct timespec ts[2]) 
{
  int res;
  char *monfs_path;
  struct timeval tv[2];

  monfs_path = get_relative_monfs_path(path);
  if (monfs_path == NULL)
    return -ENOMEM;

  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;

  res = utimes(monfs_path, tv);
  if (res == -1)
    res = -errno;

  free(monfs_path);
  return res;
}

struct 
fuse_operations monfs_oper = {
  .getattr 		= monfs_getattr,
  .readlink		= monfs_readlink,
  .mkdir        	= monfs_mkdir,
  .mknod		= monfs_mknod,
  .unlink		= monfs_unlink,
  .rmdir		= monfs_rmdir,
  .symlink		= monfs_symlink,
  .rename		= monfs_rename,
  .link			= monfs_link,
  .chmod		= monfs_chmod,
  .chown		= monfs_chown,
  .truncate		= monfs_truncate,
  .open			= monfs_open,
  .read 	        = monfs_read,
  .write		= monfs_write,
  .statfs		= monfs_statfs,
  .flush		= monfs_flush,
  .release 		= monfs_release,
  .fsync		= monfs_fsync,
#ifdef HAVE_SETXATTR
  .setxattr		= monfs_setxattr,
  .getxattr		= monfs_getxattr,
  .listxattr		= monfs_listxattr,
  .removexattr 	= monfs_removexattr,
#endif
  .opendir		= monfs_opendir,
  .readdir 		= monfs_readdir,
  .releasedir 	= monfs_releasedir,
  .init			= monfs_init,
  .destroy		= monfs_destroy,
  .access 		= monfs_access,
  .create 		= monfs_create,
  .ftruncate		= monfs_ftruncate,
  .fgetattr		= monfs_fgetattr,
#ifdef ULOCKMGR
  .lock			= monfs_lock,
#endif
  .utimens 		= monfs_utimens,
};

/**
 * Main Routine
 */
static char *program_name = "monfs";

static void
usage()
{
  const char *fusehelp[] = { program_name, "-ho", NULL };
	
  fprintf(stderr,
	  "Usage: %s [MonFS options] <mountpoint> [FUSE options]\n"
	  "\n"
	  "	MonFS options is not implemented yet."
	  "\n", program_name);
	
  fuse_main(2, (char **) fusehelp, &monfs_oper, NULL);
}

static int
next_arg_set(char **valp, int *argcp, char ***argvp, int errorexit)
{
  int argc = *argcp;
  char **argv = *argvp;
	
  --argc;
  ++argv;
  if (argc <= 0 ||
      (argc > 0 && argv[0][0] == '-' && argv[0][1] != '\0')) {
    if (errorexit) {
      usage();
      exit(1);
    } else {
      return 0;
    }
  }
		
  *valp = *argv;
  *argcp = argc;
  *argvp = argv;
	
  return 1;
}

static void
parse_long_option(int *argcp, char ***argvp)
{
  char **argv = *argvp;
	
  if (strcmp(&argv[0][1], "-nomonitor") == 0) {
    monitor_flag = 0;
  } else if (strcmp(&argv[0][1], "-db") == 0){
    next_arg_set(&db_filename, argcp, argvp, 1);

    if (!is_absolute_path(db_filename)) {
      db_filename = realpath(db_filename, NULL);
      if (db_filename == NULL) {
	usage();
	exit(1);
      }
    }

  } else {
    usage();
    exit(1);
  }	
}

static void
check_monfs_options(int *argcp, char ***argvp)
{
  int argc = *argcp;
  char **argv = *argvp;
  char *argv0 = *argv;
	
  --argc;
  ++argv;
  while (argc > 0 && argv[0][0] == '-') {

    if (argv[0][1] == '-')
      parse_long_option(&argc, &argv);
    else {
      // parse short options here.
      // not implemented yet.
      usage();
      exit(1);
    }
    --argc;
    ++argv;
  }
  ++argc;
  --argv;

  // set absolute path
  if (argc > 1 && argv[1][0] != '-') {
    if (is_absolute_path(argv[1])) { 
      monfs_root = argv[1];
    } else {
      //char abs_path[PATH_MAX+1];
      monfs_root = realpath(argv[1], NULL);
      if (monfs_root == NULL) {
	usage();
	exit(1);
      }
      argv[1] = monfs_root;
    }
  }
	
  *argcp = argc;
  *argv = argv0;
  *argvp = argv;
}

static void
set_monfs_options()
{
  fprintf(stdout, "monfs root : %s\n", monfs_root);
  fprintf(stdout, "monfs db : %s\n", db_filename);

  chdir(monfs_root);
  monfs_root_fd = open(".", 0);	
}

int
main(int argc, char *argv[])
{
  int res;

  if (argc > 0)
    program_name = basename(argv[0]);

  check_monfs_options(&argc, &argv);
  set_monfs_options();

  umask(0);
  res = fuse_main(argc, argv, &monfs_oper, NULL);

  return (res);
}
