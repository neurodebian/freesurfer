#
# Makefile.am for fsfast/toolbox
#
# here even though file list is not given, but .m is enough to discriminate files
#
foodir=$(prefix)/fsfast/toolbox
foo_DATA= $(wildcard *.m)


# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES="tdr_deghost.m tdr_fftslice.m tdr_fidb0.m tdr_fidmat2.m tdr_fidmat.m tdr_fidt2star.m tdr_frecon.m tdr_ghostcomp.m tdr_ifftslice.m tdr_intpertrap.m tdr_kshift.m tdr_lininterp.m tdr_measasc.m tdr_pedmatrix.m tdr_peshift.m tdr_phdist_est.m tdr_rcolmat.m tdr_recon_cols.m tdr_recon_fourier.m tdr_recon.m tdr_recon_rows.m tdr_recon_sim.m tdr_rotate_ktraj.m tdr_sliceorder.m tdr_smooth1d.m tdr_smooth2d.m tdr_smooth3d.m tdr_trapspect2d.m"
include $(top_srcdir)/Makefile.extra

