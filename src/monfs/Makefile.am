AM_CPPFLAGS = -I$(top_srcdir)/include -D_FILE_OFFSET_BITS=64 -D_REENTRANT
bin_PROGRAMS = monfs

monfs_SOURCES = monfs.c
monfs_LDFLAGS = -L$(top_srcdir)/src/libmonfs -lfuse -lmonfs # -lulockmgr 

