lib_LTLIBRARIES = libmonfs.la
libmonfs_la_SOURCES = monitor.c config.h config.c access_profile.h access_profile.c access_profile_queue.h access_profile_queue.c logger.h logger.c queue.h queue.c hash.h hash.c error.h error.c
libmonfs_la_CFLAGS = -Wall -I$(top_srcdir)/include -D_REENTRANT -DMONFS_CONFIG='"$(sysconfdir)/monfs.conf"'
