#
# Makefile.am for bin
#
# no file list is given. Therefore I have to check whether a file is a script or not
# 
install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/fsfast/bin; \
	d="$(DESTDIR)$(prefix)/fsfast/bin/"; \
	for p in *; do \
	val=`file $$p | grep script`;\
	if test "$$val" != "" ; then \
	  echo " $(INSTALL_SCRIPT) $$p $$d$$p"; \
	    $(INSTALL_SCRIPT) $$p $$d$$p; \
	fi; \
	done

uninstall-local:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/fsfast/bin; \
	d="$(DESTDIR)$(prefix)/fsfast/bin/"; \
	for p in *; do \
	val=`file $$p | grep script`;\
	if test "$$val" != "" ; then \
	  echo " rm -f $$d$$p"; \
	  rm -f $$d$$p; \
	fi; \
	done
 
# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES="tdr-avgechoes tdr-fidmat tdr-ghostcomp tdr-rcolmat"
include $(top_srcdir)/Makefile.extra

