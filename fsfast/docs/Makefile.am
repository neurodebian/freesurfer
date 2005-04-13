#
# Makefile.am for docs
#
foodir=$(prefix)/fsfast/docs
# I cannot specify *, since Makefile etc will be copied
foo_DATA = $(wildcard *.tex)

# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES=""
include $(top_srcdir)/Makefile.extra
