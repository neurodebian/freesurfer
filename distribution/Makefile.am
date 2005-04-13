#
# Makefile.am
#
SUBDIRS=average lib docs grad_unwarp_tables

foodir=$(prefix)
foo_DATA=surface_labels.txt tkmeditColorsCMA tkmeditParcColorsCMA Simple_surface_labels2002.txt FreeSurferEnv.csh SetUpFreeSurfer.csh


SetUpFreeSurfer.csh: SetUpFreeSurfer-SAMPLE.csh
	sed < $< -e 's,REPLACE_WITH_PREFIX,$(prefix),' > $@
	chmod a+x $@

# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES=""
include $(top_srcdir)/Makefile.extra


