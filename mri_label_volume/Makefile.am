## 
## Makefile.am 
##

AM_CFLAGS=-I$(top_srcdir)/include
AM_LDFLAGS=

bin_PROGRAMS = mri_label_volume
mri_label_volume_SOURCES=mri_label_volume.c
mri_label_volume_LDADD= $(addprefix $(top_builddir)/, $(LIBS_MGH))
mri_label_volume_LDFLAGS=$(OS_LDFLAGS)

# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES=""
include $(top_srcdir)/Makefile.extra
